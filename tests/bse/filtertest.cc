/* Tool for testing IIR filters generated by BSE
 * Copyright (C) 2006 Stefan Westerfeld
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General 
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <string>
#include <vector>
#include <set>
#include <complex>
#include <bse/bsemain.h>
#include <bse/bsemath.h>
#include <bse/bsemathsignal.h>
#include <bse/gslfilter.h>
//#define TEST_VERBOSE
#include <birnet/birnettests.h>
#include "topconfig.h"

using std::string;
using std::vector;
using std::set;
using std::min;

struct Options {
  bool                    dump_gnuplot_data;
  string		  program_name;

  Options() :
    dump_gnuplot_data (false),
    program_name ("filtertest")
  {
  }
  void parse (int *argc_p, char **argv_p[]);
} options;

static void
usage ()
{
  printf ("usage: filtertest [ <options>... ]\n");
  printf ("\n");
  printf ("When called without options, filtertest checks the compiled-in filters\n");
  printf ("against the compiled-in specifications.\n");
  printf ("\n");
  printf ("Options:\n");
  printf (" --dump-gnuplot-data    dump gnuplot data with filter transfer function\n");
  printf ("                        and specification\n");
  printf ("\n");
  printf ("Examples:\n");
  printf ("  # check filters against specification\n");
  printf ("  filtertest\n");
  printf ("  # generating and plotting gnuplot data\n");
  printf ("  filtertest --dump-gnuplot-data\n");
  printf ("  ls filtertest_*.gp\n");
  printf ("  gnuplot filtertest_bw8.gp\n");
}

static bool
check_arg (uint         argc,
           char        *argv[],
           uint        *nth,
           const char  *opt,              /* for example: --foo */
           const char **opt_arg = NULL)   /* if foo needs an argument, pass a pointer to get the argument */
{
  g_return_val_if_fail (opt != NULL, false);
  g_return_val_if_fail (*nth < argc, false);

  const char *arg = argv[*nth];
  if (!arg)
    return false;

  uint opt_len = strlen (opt);
  if (strcmp (arg, opt) == 0)
    {
      if (opt_arg && *nth + 1 < argc)     /* match foo option with argument: --foo bar */
        {
          argv[(*nth)++] = NULL;
          *opt_arg = argv[*nth];
          argv[*nth] = NULL;
          return true;
        }
      else if (!opt_arg)                  /* match foo option without argument: --foo */
        {
          argv[*nth] = NULL;
          return true;
        }
      /* fall through to error message */
    }
  else if (strncmp (arg, opt, opt_len) == 0 && arg[opt_len] == '=')
    {
      if (opt_arg)                        /* match foo option with argument: --foo=bar */
        {
          *opt_arg = arg + opt_len + 1;
          argv[*nth] = NULL;
          return true;
        }
      /* fall through to error message */
    }
  else
    return false;

  usage();
  exit (1);
}

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i;

  g_return_if_fail (argc >= 0);

  for (i = 1; i < argc; i++)
    {
      //const char *opt_arg;
      if (strcmp (argv[i], "--help") == 0 ||
          strcmp (argv[i], "-h") == 0)
        {
          usage();
          exit (0);
        }
      else if (strcmp (argv[i], "--version") == 0 ||
               strcmp (argv[i], "-v") == 0)
        {
          printf ("%s %s\n", program_name.c_str(), BSE_VERSION);
          exit (0);
        }
      else if (check_arg (argc, argv, &i, "--dump-gnuplot-data"))
	{
	  dump_gnuplot_data = true;
	}
    }

  /* resort argc/argv */
  guint e = 1;
  for (i = 1; i < argc; i++)
    if (argv[i])
      {
        argv[e++] = argv[i];
        if (i >= e)
          argv[i] = NULL;
      }
  *argc_p = e;
}

class FilterTest
{
public:
  enum TestMode
  {
    TEST_NOTHING = 0,
    TEST_COMPUTED_RESPONSE = 1,
    TEST_SCANNED_RESPONSE = 2,
    TEST_COMPUTED_AND_SCANNED_RESPONSE = TEST_COMPUTED_RESPONSE | TEST_SCANNED_RESPONSE
  };

private:
  string         m_name;
  guint          m_order;
  TestMode       m_test_mode;
  vector<double> m_a, m_b;
  set<double>    m_gp_arrows;
  set<double>    m_gp_lines;

  static const double FS = 10000.0;
  static const double delta_f = 10000.0 / 2000.0; //997; /* use a prime number to decrease chance of aliasing with mixfreq */
  static const double MIN_DB = -1000;
  static const double DB_EPSILON = 0.01;  /* for comparisions */

  double
  response (double f)
  {
    double u = 2.0 * PI * f / FS;
    std::complex<double> z (cos (u), sin (u)); /* exp( j omega T ) */

    guint o = m_order;
    std::complex<double> num = m_a[o], den = m_b[o]; 
    while (o--)
      {
	num = num * z + m_a[o];
	den = den * z + m_b[o];
      }
    std::complex<double> w = num / den;
    return abs (w);
  }

  double
  scan_response (double f)
  {
    const double MAX_SCAN_FREQ = FS / 2 - 0.01;
    f = min (f, MAX_SCAN_FREQ);
    return gsl_filter_sine_scan (m_order, &m_a[0], &m_b[0], f / FS * 2 * PI, 10000);
  }

public:
  FilterTest (const char   *name,
	      guint         order,
              const double *coefficients,
	      TestMode      test_mode = TEST_COMPUTED_AND_SCANNED_RESPONSE) :
    m_name (name),
    m_order (order),
    m_test_mode (test_mode)
  {
    TSTART("%s, Order %d", name, order);

    for (guint i = 0; i < m_order * 2 + 2; i += 2)
      {
	m_b.push_back (coefficients[i]);
	m_a.push_back (coefficients[i+1]);
      }
  }
  ~FilterTest()
  {
    TDONE();
  }
  void
  check_response_db (double freq,
                     double min_resp_db,
		     double max_resp_db)
  {
    if (m_test_mode & TEST_COMPUTED_RESPONSE)
      {
	double resp = bse_db_from_factor (response (freq), MIN_DB);
	if (!(resp > min_resp_db - DB_EPSILON) || !(resp < max_resp_db + DB_EPSILON))
	  {
	    g_printerr ("\n*** check_response_db: computed response at frequency %f is %f\n", freq, resp);
	    g_printerr ("*** check_response_db: but should be in interval [%f..%f]\n", min_resp_db, max_resp_db);
	  }
	TASSERT (resp > min_resp_db - DB_EPSILON);
	TASSERT (resp < max_resp_db + DB_EPSILON);
      }

    if (m_test_mode & TEST_SCANNED_RESPONSE)
      {
	double scan_resp = bse_db_from_factor (scan_response (freq), MIN_DB);
	if (!(scan_resp > min_resp_db - DB_EPSILON) || !(scan_resp < max_resp_db + DB_EPSILON))
	  {
	    g_printerr ("\n*** check_response_db: scanned response at frequency %f is %f\n", freq, scan_resp);
	    g_printerr ("*** check_response_db: but should be in interval [%f..%f]\n", min_resp_db, max_resp_db);
	  }
	TASSERT (scan_resp > min_resp_db - DB_EPSILON);
	TASSERT (scan_resp < max_resp_db + DB_EPSILON);
      }
  }
  void
  check_band (double	freq_start,
	      double	freq_end,
	      double    min_resp_db,
	      double	max_resp_db)
  {
    g_return_if_fail (freq_start <= freq_end);
    g_return_if_fail (freq_end <= FS/2);

    /* we don't need to actually perform the checks just to dump the gnuplot data */
    if (options.dump_gnuplot_data)
      return;

    TPRINT ("checking band: response in interval [%f..%f] should be in interval [%f..%f] dB\n",
	    freq_start, freq_end, min_resp_db, max_resp_db);

    for (double f = freq_start; f < freq_end; f += delta_f)
      check_response_db (f, min_resp_db, max_resp_db);

    if (freq_start != freq_end)
      check_response_db (freq_end, min_resp_db, max_resp_db);
  }
  void
  check_passband (double freq_start,
		  double freq_end,
		  double ripple_db)
  {
    m_gp_arrows.insert (freq_start);
    m_gp_arrows.insert (freq_end);
    m_gp_lines.insert (ripple_db);

    check_band (freq_start, freq_end, ripple_db, 0);
  }
  void
  check_stopband (double freq_start,
		  double freq_end,
		  double ripple_db)
  {
    m_gp_arrows.insert (freq_start);
    m_gp_arrows.insert (freq_end);
    m_gp_lines.insert (ripple_db);

    check_band (freq_start, freq_end, MIN_DB, ripple_db);
  }
  /**
   * creates two files, a datafile named filename_prefix ".data" and a gnuplot
   * script called filename_prefix ".gp", which can be used to plot the filter,
   * including the specification checks
   */
  bool
  dump_gp (const string& filename_prefix)
  {
    if (!options.dump_gnuplot_data)
      return false;

    FILE *data_file = fopen ((filename_prefix + ".data").c_str(), "w");
    if (!data_file)
      return false;
    FILE *gp_file = fopen ((filename_prefix + ".gp").c_str(), "w");
    if (!gp_file)
      {
	fclose (data_file);
	return false;
      }
    g_printerr ("creating gnuplot files '%s.gp', '%s.data'", filename_prefix.c_str(), filename_prefix.c_str());

    for (double f = 0; f < FS/2; f += delta_f)
      {
	fprintf (data_file, "%f %f %f\n", f, bse_db_from_factor (response (f), -1000),
					     bse_db_from_factor (scan_response (f), -1000));
      }
    fprintf (gp_file, "# Test order=%u  norm=%f:\n", m_order,
	bse_poly_eval (m_order, &m_a[0], 1) / bse_poly_eval (m_order, &m_b[0], 1));
    /* we don't need H(z) that because we do it ourselves, but we can print it
     * in case somebody wants to play around with gnuplot
     */
    fprintf (gp_file, "H(z)=%s/%s\n", bse_poly_str (m_order, &m_a[0], "z"),
				      bse_poly_str (m_order, &m_b[0], "z"));
    fprintf (gp_file, "load '../../bse/tests/filter-defs.gp'\n");
    fprintf (gp_file, "call '../../bse/tests/arrows.gp' %d", m_gp_arrows.size());
    for (set<double>::iterator ai = m_gp_arrows.begin(); ai != m_gp_arrows.end(); ai++)
      fprintf (gp_file, " %f", *ai);
    fprintf (gp_file, "\n");
    fprintf (gp_file, "plot 0, '%s.data' using ($1):($3), '%s.data' using ($1):($2) with lines",
		      filename_prefix.c_str(), filename_prefix.c_str());
    for (set<double>::iterator li = m_gp_lines.begin(); li != m_gp_lines.end(); li++)
      fprintf (gp_file, ", %f", *li);
    fprintf (gp_file, "\n");
    fprintf (gp_file, "pause -1\n");

    fclose (gp_file);
    fclose (data_file);
    return 0;
  }
};

int
main (int     argc,
      char  **argv)
{
  bse_init_test (&argc, &argv, NULL);
  options.parse (&argc, &argv);

  {
    //! ORDER=8 MIX_FREQ=10000 PASS1=2000 yet.sh blp
    const double coeffs[] = {	  // BSEcxxmgc
        +1.00000000000000000000E+00,    +2.27184001245132058747E-03,	// BSEcxxmgc
        -1.59056649578489484043E+00,    +1.81747200996105646997E-02,	// BSEcxxmgc
        +2.08381330026821487422E+00,    +6.36115203486369712449E-02,	// BSEcxxmgc
        -1.53262556329449450843E+00,    +1.27223040697273942490E-01,	// BSEcxxmgc
        +8.69440915484917642431E-01,    +1.59028800871592435051E-01,	// BSEcxxmgc
        -3.19175943252755334179E-01,    +1.27223040697273942490E-01,	// BSEcxxmgc
        +8.20901315715001494988E-02,    +6.36115203486369712449E-02,	// BSEcxxmgc
        -1.22466701861471700258E-02,    +1.81747200996105646997E-02,	// BSEcxxmgc
        +8.61368381197359644919E-04,    +2.27184001245132058747E-03,	// BSEcxxmgc
    };  // BSEcxxmgc

    FilterTest bw8 ("Butterworth Lowpass 2000 Hz", 8, coeffs);
    bw8.check_passband (0, 2000, bse_db_from_factor (1/sqrt(2), -30));
    bw8.check_stopband (3500, 5000, -68);
    bw8.dump_gp ("filtertest_bw8");
  }
  {
    //! ORDER=12 MIX_FREQ=10000 PASS1=2000 PASS_RIPPLE=0.5 STOP_DB=-96 yet.sh elpd
    const double coeffs[] = {	  // BSEcxxmgc
        +1.00000000000000000000E+00,    +1.05368736204784784784E-03,	// BSEcxxmgc
        -5.52088177597197571345E+00,    +1.77484258573414174429E-03,	// BSEcxxmgc
        +1.72810439673491735846E+01,    +5.59320286089960705400E-03,	// BSEcxxmgc
        -3.71963611365582025314E+01,    +8.10769525850848354898E-03,	// BSEcxxmgc
        +6.03154515939285431614E+01,    +1.32251577282920321221E-02,	// BSEcxxmgc
        -7.64932090088492628865E+01,    +1.54420099348586276067E-02,	// BSEcxxmgc
        +7.73512884100554884981E+01,    +1.73653659064273678536E-02,	// BSEcxxmgc
        -6.26072020236414132910E+01,    +1.54420099348586293414E-02,	// BSEcxxmgc
        +4.02479481049599954190E+01,    +1.32251577282920355916E-02,	// BSEcxxmgc
        -2.00855297261297387479E+01,    +8.10769525850848701842E-03,	// BSEcxxmgc
        +7.43087292602526794383E+00,    +5.59320286089960792136E-03,	// BSEcxxmgc
        -1.85052119790466007565E+00,    +1.77484258573414174429E-03,	// BSEcxxmgc
        +2.41243520047625281677E-01,    +1.05368736204784763100E-03,	// BSEcxxmgc
    };  // BSEcxxmgc

    FilterTest ell12 ("Elliptic Lowpass 2000 Hz", 12, coeffs);
    ell12.check_passband (0, 2000, -0.5);

    /* FIXME: we should get better results (= -96 dB) if we implement this filter with cascading lowpasses */
    ell12.check_stopband (2160, 5000, -95.5);
    ell12.dump_gp ("filtertest_ell12");
  }
  {
    //! ORDER=7 MIX_FREQ=10000 PASS1=600 PASS_RIPPLE=0.1 yet.sh chp
    const double coeffs[] = {	  // BSEcxxmgc
        +1.00000000000000000000E+00,    +3.56249822046611874793E-01,	// BSEcxxmgc
        -4.96234086898923099085E+00,    -2.49374875432628329008E+00,	// BSEcxxmgc
        +1.07914114429296752462E+01,    +7.48124626297884898207E+00,	// BSEcxxmgc
        -1.32536434961142131073E+01,    -1.24687437716314164504E+01,	// BSEcxxmgc
        +9.89084598843435891524E+00,    +1.24687437716314164504E+01,	// BSEcxxmgc
        -4.46412696164509537056E+00,    -7.48124626297884898207E+00,	// BSEcxxmgc
        +1.11977829190352773381E+00,    +2.49374875432628329008E+00,	// BSEcxxmgc
        -1.17830171950224868449E-01,    -3.56249822046611874793E-01,	// BSEcxxmgc
    };  // BSEcxxmgc

    FilterTest chp7 ("Chebychev Highpass 600 Hz", 7, coeffs);
    chp7.check_passband (600, 5000, -0.1);
    chp7.check_stopband (0, 250, -70);
    chp7.dump_gp ("filtertest_chp7");
  }
  {
    //! ORDER=9 MIX_FREQ=10000 PASS1=1500 PASS2=3500 yet.sh bbp
    const double coeffs[] = {	  // BSEcxxmgc
        +1.00000000000000000000E+00,    +1.06539452359780476010E-03,	// BSEcxxmgc
        -8.88178419700125232339E-16,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +1.79158135278859775852E+00,    -9.58855071238024284086E-03,	// BSEcxxmgc
        -1.88737914186276611872E-15,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +2.53189988089812256788E+00,    +3.83542028495209713634E-02,	// BSEcxxmgc
        -2.77555756156289135106E-15,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +2.11822942034193495431E+00,    -8.94931399822155998480E-02,	// BSEcxxmgc
        -1.66533453693773481064E-15,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +1.37075629439323409819E+00,    +1.34239709973323406711E-01,	// BSEcxxmgc
        -8.88178419700125232339E-16,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +6.09038913076474175412E-01,    -1.34239709973323406711E-01,	// BSEcxxmgc
        -3.88578058618804789148E-16,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +1.99331556962956374379E-01,    +8.94931399822155998480E-02,	// BSEcxxmgc
        -1.04083408558608425665E-16,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +4.31047310152814222572E-02,    -3.83542028495209713634E-02,	// BSEcxxmgc
        -1.30104260698260532081E-17,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +5.80426165430881872698E-03,    +9.58855071238024284086E-03,	// BSEcxxmgc
        -9.75781955236953990607E-19,    +0.00000000000000000000E+00,	// BSEcxxmgc
        +3.55580604257624494427E-04,    -1.06539452359780476010E-03,	// BSEcxxmgc
    };  // BSEcxxmgc

    FilterTest bbp18 ("Butterworth Bandpass 1500-3500 Hz", 18, coeffs);
    bbp18.check_passband (1500, 3500, bse_db_from_factor (1/sqrt(2), -30));
    bbp18.check_stopband (0, 1000, -49.5);
    bbp18.check_stopband (4000, 5000, -49.5);
    bbp18.dump_gp ("filtertest_bbp18");
  }
  {
    //! ORDER=2 MIX_FREQ=10000 PASS1=300 PASS2=1000 PASS_RIPPLE=2.5 STOP1_EDGE=400 yet.sh ebsh
    const double coeffs[] = {	  // BSEcxxmgc
        +1.00000000000000000000E+00,    +6.05473613799674903468E-01,	// BSEcxxmgc
        -3.29583926802918547594E+00,    -2.26195685035092974857E+00,	// BSEcxxmgc
        +4.34131120422883576282E+00,    +3.32150977307268702177E+00,	// BSEcxxmgc
        -2.73689660911365040263E+00,    -2.26195685035092974857E+00,	// BSEcxxmgc
        +7.02817346525631658771E-01,    +6.05473613799674903468E-01,	// BSEcxxmgc
    };  // BSEcxxmgc
    FilterTest ebsh4 ("Elliptic Bandstop 300-1000 Hz", 4, coeffs);
    ebsh4.check_passband (0, 300, -2.5);
    ebsh4.check_stopband (400, 755, -21.5);
    ebsh4.check_passband (1000, 5000, -2.5);
    ebsh4.dump_gp ("filtertest_ebsh4");
  }
}

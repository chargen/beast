// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include <bse/bsemain.hh>
#include <bse/bseserver.hh>
#include <sys/resource.h>
#include <unordered_map>
#include <unistd.h>
#include <stdio.h>

using namespace Bse;

// == arg parsing ==
static bool verbose = true;
#define printq(...)     do { if (verbose) printout (__VA_ARGS__); } while (0)

struct ArgDescription {
  const char *arg_name, *value_name, *arg_blurb;
  String value;
};

class ArgParser {
  const size_t                                n_args_;
  ArgDescription                             *const args_;
  std::unordered_map<String, ArgDescription*> names_;
  void     parse_args (const size_t N, const ArgDescription *adescs);
public:
  template<size_t N>
  explicit ArgParser  (ArgDescription (&adescs) [N]) : n_args_ (N), args_ (adescs) {}
  String   parse_args (const uint argc, char *const argv[]); // returns error message
  String   operator[] (const String &arg_name) const;
};

String
ArgParser::operator[] (const String &arg_name) const
{
  auto it = names_.find (arg_name);
  if (it == names_.end())
    return "";
  ArgDescription *adesc = it->second;
  return adesc->value;
}

String
ArgParser::parse_args (const uint argc, char *const argv[])
{
  std::unordered_map<String, String> aliases;
  std::vector<String> fixed; // fixed arguments, like <OBLIGATION> or [OPTIONAL]
  size_t need_obligatory = 0;
  // fill names_ from args_
  for (size_t i = 0; i < n_args_; i++)
    {
      StringVector names = string_split (args_[i].arg_name, ",");
      for (size_t j = 0; j < names.size(); j++)
        {
          String name = string_canonify (names[j], string_set_a2z() + string_set_A2Z() + "0123456789_-", "");
          while (name[0] == '-')
            name = name.substr (1);
          if (name.empty())
            continue;
          if (j == 0)
            {
              names_[name] = &args_[i];
              if (names[j][0] == '<')
                {
                  need_obligatory++;
                  fixed.push_back (name);
                }
              else if (names[j][0] == '[')
                fixed.push_back (name);
            }
          else // found an alias after ','
            names_[name] = &args_[i];
        }
    }
  // parse and assign command line arguments
  bool seen_dashdash = false;
  size_t fixed_index = 0;
  for (size_t i = 0; i < argc; i++)
    if (!argv[i])
      continue;
    else if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0)
      seen_dashdash = true;
    else if (!seen_dashdash && argv[i][0] == '-')
      {
        const char *arg = argv[i] + 1 + (argv[i][1] == '-');
        const char *eq = strchr (arg, '=');
        String arg_name = !eq ? arg : String (arg, eq - arg);
        auto it = names_.find (arg_name);
        if (it == names_.end())
          return string_format ("invalid argument: %s", argv[i]);
        ArgDescription *adesc = it->second;
        if (adesc->value_name && adesc->value_name[0])
          {
            if (eq)
              adesc->value = eq + 1;
            else
              {
                if (i + 1 >= argc)
                  return string_format ("incomplete argument: %s", argv[i]); // argument lacks value assignment
                i++;
                adesc->value = argv[i];
              }
          }
        else
          adesc->value = "1"; // value means 'option is present'
      }
    else // non-option arguments
      {
        if (fixed_index >= fixed.size())
          return string_format ("invalid extra argument: %s", argv[i]);
        ArgDescription *adesc = names_[fixed[fixed_index]];
        adesc->value = argv[i];
        fixed_index++;
      }
  if (fixed_index < need_obligatory)
    return string_format ("missing mandatory argument <%s>", fixed[fixed_index]);
  return ""; // success
}

// == crawl ==
static ArgDescription crawl_options[] = {
  { "<glob>",   "",     "Glob pattern to list matching files", "" },
};

static String
crawl (const ArgParser &ap)
{
  const String glob = ap["glob"];
  // plist = bse_search_path_list_files (argv[1], NULL);
  SfiRing *ring = sfi_file_crawler_list_files (glob.c_str(), NULL, GFileTest (0));
  while (ring)
    {
      const char *name = (const char*) sfi_ring_pop_head (&ring);
      printout ("%s\n", name);
    }
  sfi_ring_free_deep (ring, g_free);
  return "";
}


// == render2wav ==
static ArgDescription render2wav_options[] = {
  { "-s, --seconds", "<seconds>", "Number of seconds to record", "0" },
  { "<bse-file>",    "",          "The BSE file for audio rendering", "" },
  { "<wav-file>",    "",          "The WAV file to use for audio output", "" },
};

static ErrorType
render2wav (const ArgParser &ap)
{
  const String bsefile = ap["bse-file"];
  const String wavfile = ap["wav-file"];
  const double n_seconds = string_to_double (ap["seconds"]);
  auto project = BSE_SERVER.create_project (bsefile);
  project->auto_deactivate (0);
  auto err = project->restore_from_file (bsefile);
  if (err)
    return err;
  BSE_SERVER.start_recording (wavfile, n_seconds);
  err = project->play();
  printq ("Recording %s to %s...\n", bsefile, wavfile);
  printq (".");
  int counter = 0;
  while (project->is_playing())
    {
      if (counter == 0)
        printq ("\b*");
      else if (counter == 25)
        printq ("\bo");
      if (g_main_context_pending (bse_main_context))
        g_main_context_iteration (bse_main_context, false);
      else
        usleep (10 * 1000);
      counter = (counter + 1) % 50;
    }
  printq ("\n");

  return ERROR_NONE;
}

// == check-load ==
static ArgDescription check_load_options[] = {
  { "<bse-file>",    "",          "The BSE file to load and check for validity", "" },
};

static String
check_load (const ArgParser &ap)
{
  const String bsefile = ap["bse-file"];
  auto project = BSE_SERVER.create_project (bsefile);
  project->auto_deactivate (0);
  auto err = project->restore_from_file (bsefile);
  if (err)
    return string_format ("%s: loading failed: %s", bsefile, bse_error_blurb (err));
  // success
  return "";
}

// == bse tool ==
static ArgDescription bsetool_options[] = {
  { "--bse-no-load", "", "Prevent automated plugin and script registration", "" },
  { "--quiet",       "", "Prevent progress output", "" },
};

int
main (int argc_int, char *argv[])
{
  bse_init_inprocess (&argc_int, argv, "bsetool"); // Bse::cstrings_to_vector (NULL)
  const unsigned int argc = argc_int;
  // now that the BSE thread runs, drop scheduling priorities if we have any
  setpriority (PRIO_PROCESS, getpid(), 0);
  // pre-command option argument parsing
  size_t option_argc = 1; // skip argv[0]
  while (option_argc < argc && argv[option_argc][0] == '-')
    option_argc++;
  ArgParser toolap (bsetool_options);
  String error = toolap.parse_args (option_argc - 1, argv + 1); // skip argv[0]
  if (!error.empty())
    {
      printerr ("%s: %s\n", argv[0], error);
      return 127;
    }
  verbose = !string_to_bool (toolap["quiet"]);
  // load BSE plugins, scripts, ladspa plugins, etc
  if (!string_to_bool (toolap["bse-no-load"]))
    {
      BSE_SERVER.register_core_plugins();
      while (g_main_context_pending (bse_main_context))
        g_main_context_iteration (bse_main_context, false);
    }
  // command parsing
  if (option_argc < argc && argv[option_argc] == String ("render2wav"))
    {
      ArgParser ap (render2wav_options);
      String error = ap.parse_args (argc - option_argc - 1, argv + option_argc + 1);
      if (!error.empty())
        {
          printerr ("%s: render2wav: %s\n", argv[0], error);
          return 127;
        }
      ErrorType err = render2wav (ap);
      if (err != ERROR_NONE)
        {
          printerr ("%s: render2wav: %s\n", argv[0], bse_error_blurb (err));
          return 127;
        }
      return 0; // success
    }
  else if (option_argc < argc && argv[option_argc] == String ("check-load"))
    {
      ArgParser ap (check_load_options);
      String error = ap.parse_args (argc - option_argc - 1, argv + option_argc + 1);
      if (!error.empty())
        {
          printerr ("%s: check-load: %s\n", argv[0], error);
          return 127;
        }
      error = check_load (ap);
      if (!error.empty())
        {
          printerr ("check-load: %s\n", error);
          return 127;
        }
      return 0; // success
    }
  else if (option_argc < argc && argv[option_argc] == String ("crawl"))
    {
      ArgParser ap (crawl_options);
      String error = ap.parse_args (argc - option_argc - 1, argv + option_argc + 1);
      if (!error.empty())
        {
          printerr ("%s: crawl: %s\n", argv[0], error);
          return 127;
        }
      error = crawl (ap);
      if (!error.empty())
        {
          printerr ("crawl: %s\n", error);
          return 127;
        }
      return 0; // success
    }
  else
    {
      printerr ("%s: %s\n", argv[0], "missing command");
      return 127;
    }
}

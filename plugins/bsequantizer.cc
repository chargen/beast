// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "bsequantizer.genidl.hh"
#include <bse/bseengine.hh>
#include <bse/bsemathsignal.hh>

namespace Bse {
namespace Standard {

class Quantizer : public QuantizerBase {
  class Module: public SynthesisModule {
    /* params */
    double qfactor;
  public:
    void
    config (QuantizerProperties *params)
    {
      qfactor = params->qsteps;
    }
    void
    reset ()
    {}
    void
    process (unsigned int n_values)
    {
      const double ifactor = 1.0 / qfactor;
      /* use of nearbyint() relies on the FPU rounding mode round-to-nearest (guaranteed by BSE) */
      if (ostream (OCHANNEL_AUDIO_OUT1).connected)
        {
          if (istream (ICHANNEL_AUDIO_IN1).connected)
            {
              float *ovalues = ostream (OCHANNEL_AUDIO_OUT1).values;
              const float *ivalues = istream (ICHANNEL_AUDIO_IN1).values;
              for (uint i = 0; i < n_values; i++)
                ovalues[i] = ifactor * nearbyint (ivalues[i] * qfactor);
            }
          else
            ostream_set (OCHANNEL_AUDIO_OUT1, const_values (0));
        }
      if (ostream (OCHANNEL_AUDIO_OUT2).connected)
        {
          if (istream (ICHANNEL_AUDIO_IN2).connected)
            {
              float *ovalues = ostream (OCHANNEL_AUDIO_OUT2).values;
              const float *ivalues = istream (ICHANNEL_AUDIO_IN2).values;
              for (uint i = 0; i < n_values; i++)
                ovalues[i] = ifactor * nearbyint (ivalues[i] * qfactor);
            }
          else
            ostream_set (OCHANNEL_AUDIO_OUT2, const_values (0));
        }
    }
  };
public:
  bool
  property_changed (QuantizerPropertyID prop_id)
  {
    switch (prop_id)
      {
      case PROP_QSTEPS:
        qstep_powers = log (qsteps) / BSE_LN2;
        notify ("qstep_powers");
        break;
      case PROP_QSTEP_POWERS: /* GUI only property */
        qsteps = pow (2, qstep_powers);
        qstep_powers = log (qsteps) / BSE_LN2; /* re-quantise */
        notify ("qsteps");
        break;
      default: ;
      }
    return false;
  }
public:
  /* implement creation and config methods for synthesis Module */
  BSE_EFFECT_INTEGRATE_MODULE (Quantizer, Module, QuantizerProperties);
};

BSE_CXX_DEFINE_EXPORTS();
BSE_CXX_REGISTER_ALL_TYPES_FROM_BSEQUANTIZER_IDL();

} } // Bse::Standard

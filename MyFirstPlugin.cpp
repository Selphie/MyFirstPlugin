#include "MyFirstPlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

MyFirstPlugin::MyFirstPlugin(const InstanceInfo& info) // Konstruktor
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets)) // legt fest, wie viele Parameter und Presets es gibt
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%"); // Initialisiert einen Parameter vom Typ Double
  GetParam(kDrive)->InitDouble("Drive", 0., 0., 100.0, 0.01, "%"); // Initialisiert einen Parameter vom Typ Double


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() { // erstellt das Fenster und das Canvas
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) { // definiert alle Controls und Layouts in der GUI
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain, "Gain"));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(100), kDrive, "Clipping"));
  };
#endif
}

// Linear → dB
float linearToDb(float x)
{
  const float epsilon = 1e-20f; // verhindert log(0)
  return 20.0f * std::log10f(std::max(std::abs(x), epsilon));
}

//dB → Linear (mit Vorzeichen)
float dbToLinear(float dB, float originalSign = 1.0f) { return std::copysign(std::pow(10.0f, dB / 20.0f), originalSign); }


#if IPLUG_DSP
void MyFirstPlugin::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const double drive = GetParam(kDrive)->Value() / 100.;
  const int nChans = NOutChansConnected();
  double phase = 0.0;
  
   for (int s = 0; s < nFrames; s++)
  {                                  // nFrames beschriebt die Anzahl Samples im aktuellen Audio-Block

    float testSample = sinf(phase);
    phase += 2.0f * M_PI * 440.0f / (float)GetSampleRate(); // 440 Hz Sinus
    if (phase > 2.0f * M_PI)
      phase -= 2.0f * M_PI;

    for (int c = 0; c < nChans; c++) // nChans beschreibt die Anzahl der Kanäle
    {
      // double sample = inputs[c][s];
      double sample = testSample; // Ausgang = Testsignal
      sample *= gain;  // zuerst Gain
      // sample *= drive; // dann Drive (oder deine Distortion-Funktion)
      // outputs[c][s] = sample;

      float db_sample = linearToDb(sample);
      DBGMSG("Sample in DB: %f\n", db_sample);

      if (db_sample > -10.0)
      {
        db_sample = 10.0;
        DBGMSG("signal clipped");
      }

      outputs[c][s] = dbToLinear(db_sample);






    }
  }
}
#endif

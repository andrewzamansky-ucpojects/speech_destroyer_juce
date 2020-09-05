#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
						public juce::ChangeListener,
						private juce::Timer
{
public:
    //==============================================================================
    MainComponent();

    ~MainComponent() override;


    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;

    void resized() override;


private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    static juce::String getListOfActiveBits (const juce::BigInteger& b)
    {
        juce::StringArray bits;

        for (auto i = 0; i <= b.getHighestBit(); ++i)
            if (b[i])
                bits.add (juce::String (i));

        return bits.joinIntoString (", ");
    }
    void timerCallback() override;
    void dumpDeviceInfo();

    void logMessage (const juce::String& m);

    //==========================================================================
    juce::Random random;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::Label cpuUsageLabel;
    juce::Label cpuUsageText;
    juce::TextEditor diagnosticsBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

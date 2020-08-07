
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <memory>

FunFilterAudioProcessor::FunFilterAudioProcessor() noexcept
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

FunFilterAudioProcessor::~FunFilterAudioProcessor() noexcept = default;

const juce::String FunFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FunFilterAudioProcessor::acceptsMidi() const
{
    return false;
}

bool FunFilterAudioProcessor::producesMidi() const
{
    return false;
}

bool FunFilterAudioProcessor::isMidiEffect() const
{
    return false;
}

double FunFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FunFilterAudioProcessor::getNumPrograms()
{
    return 1;
}

int FunFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FunFilterAudioProcessor::setCurrentProgram([[maybe_unused]] int index)
{
}

const juce::String FunFilterAudioProcessor::getProgramName([[maybe_unused]] int index)
{
    return {};
}

void FunFilterAudioProcessor::changeProgramName(
    [[maybe_unused]] int index, [[maybe_unused]] const juce::String& newName)
{
}

void FunFilterAudioProcessor::prepareToPlay(double newSampleRate,
                                            [[maybe_unused]] int samplesPerBlock)
{
    sampleRate = newSampleRate;
    nbSamplesLeftBeforeNextStep = 0;
    currentFrequencyIndex = 0;
    nbSamplesLeftBeforeNextStep = filterChoregraphyStepPeriod;
    broadcaster.setValue<ValueIds::filterCutoff>(frequencies[currentFrequencyIndex]);
    filter.setFilterCutoffFrequency(frequencies[currentFrequencyIndex]);
    filter.setSampleRate(newSampleRate);
}

void FunFilterAudioProcessor::releaseResources()
{
}

void FunFilterAudioProcessor::nextFilterFrequency() noexcept
{
    currentFrequencyIndex = (currentFrequencyIndex + 1) % frequencies.size();
    assert(currentFrequencyIndex < frequencies.size());
    filter.setFilterCutoffFrequency(frequencies[currentFrequencyIndex]);
    broadcaster.setValue<ValueIds::filterCutoff>(frequencies[currentFrequencyIndex]);
    nbSamplesLeftBeforeNextStep = filterChoregraphyStepPeriod;
}

bool FunFilterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

int FunFilterAudioProcessor::calculateChoregraphyPeriodInSamplesFromBpm(int bpm) noexcept
{
    constexpr auto secInOneMinute = 60;
    const auto songFreqHz = bpm / secInOneMinute;
    const auto songPeriodInSamples = sampleRate / songFreqHz;
    const auto choregraphyPeriodInSamples =
        songPeriodInSamples * choregraphyLengthInBeats;
    const auto filterChoregraphySteps = static_cast<double>(frequencies.size());
    return static_cast<int>(choregraphyPeriodInSamples / filterChoregraphySteps);
}

void FunFilterAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto* playHead = getPlayHead();
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info))
        {
            const auto newFilterChoregraphyStepPeriod =
                calculateChoregraphyPeriodInSamplesFromBpm(info.bpm);
            if (newFilterChoregraphyStepPeriod != filterChoregraphyStepPeriod)
            {
                filterChoregraphyStepPeriod = newFilterChoregraphyStepPeriod;
                nbSamplesLeftBeforeNextStep = filterChoregraphyStepPeriod;
            }
        }
    }

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    auto nbProcessedSamples{0};
    while (nbProcessedSamples < buffer.getNumSamples())
    {
        const auto nbSamplesLeftToProcess = buffer.getNumSamples() - nbProcessedSamples;
        const auto nbSamplesToProcess =
            std::min(nbSamplesLeftBeforeNextStep, nbSamplesLeftToProcess);

        if (nbSamplesLeftBeforeNextStep == 0)
        {
            nextFilterFrequency();
        }

        filter.process(buffer, nbProcessedSamples, nbSamplesLeftToProcess);

        nbProcessedSamples += nbSamplesToProcess;
        nbSamplesLeftBeforeNextStep -= nbSamplesToProcess;
        assert(nbSamplesLeftBeforeNextStep >= 0);
        assert(nbProcessedSamples <= buffer.getNumSamples());
    }
}

bool FunFilterAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FunFilterAudioProcessor::createEditor()
{
    return std::make_unique<FunFilterEditor>(*this, broadcaster).release();
}

void FunFilterAudioProcessor::getStateInformation([
    [maybe_unused]] juce::MemoryBlock& destData)
{
}

void FunFilterAudioProcessor::setStateInformation([[maybe_unused]] const void* data,
                                                  [[maybe_unused]] int sizeInBytes)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return std::make_unique<FunFilterAudioProcessor>().release();
}

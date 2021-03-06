
#include "PluginEditor.h"
#include "ColorPalette.h"
#include "PluginProcessor.h"
#include "UiBroadcaster.h"

FunFilterEditor::FunFilterEditor(FunFilterAudioProcessor& p,
                                 UiBroadcaster& broadcaster) noexcept
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
    , cutoffVisualizer(broadcaster)
    , resonanceKnob(broadcaster, p.getParameterFromName("FilterResonance"))
    , rateKnob(broadcaster, p.getParameterFromName("Rate"))
    , cutoffSequence(broadcaster, p)
{
    addAndMakeVisible(cutoffVisualizer);
    addAndMakeVisible(resonanceKnob);
    addAndMakeVisible(rateKnob);
    addAndMakeVisible(cutoffSequence);

    setSize(400, 350);
}

FunFilterEditor::~FunFilterEditor() = default;

void FunFilterEditor::paint(juce::Graphics& g)
{
    g.fillAll(ColorPalette::darkSlateBlue);
}

void FunFilterEditor::resized()
{
    auto areaLeftToFill = getLocalBounds();
    cutoffVisualizer.setBounds(areaLeftToFill.removeFromTop(proportionOfHeight(0.6f))
                                   .reduced(proportionOfWidth(0.05f)));
    auto knobsArea =
        areaLeftToFill.removeFromTop(areaLeftToFill.proportionOfHeight(0.4f));
    resonanceKnob.setBounds(knobsArea.removeFromLeft(proportionOfWidth(0.5f))
                                .expanded(proportionOfHeight(0.02f)));
    rateKnob.setBounds(knobsArea.expanded(proportionOfHeight(0.02f)));

    cutoffSequence.setBounds(areaLeftToFill);
}

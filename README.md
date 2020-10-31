[Lilac Loop](https://library.vcvrack.com/LilacLoop) is a plugin for [VCV Rack](https://vcvrack.com/). As of [v1.0.0](https://github.com/grough/lilac-loop-vcv/releases/tag/v1.0.0) it contains just one module called _Looper_. If you'd like to report a bug or are having trouble using the plugin, please submit an [issue](https://github.com/grough/lilac-loop-vcv/issues).

## Module: _Looper_

The Looper module emulates the recording style of a [live looping pedal](https://en.wikipedia.org/wiki/Live_looping). It allows you to record, play and overdub a mono or polyphonic signal using a multi-function toggle control comparable to the main foot switch on a looper pedal.

[![An example patch showing how to record and loop a monophonic input signal](examples/lilac-looper-mono-example.png)](https://patchstorage.com/lilac-looper-mono-example/)

### Get Started

[Install the plugin](https://library.vcvrack.com/LilacLoop) and try one of the example patches:

- [Basic Looping](https://patchstorage.com/lilac-looper-mono-example/) (seen above)
- [Stereo looping](https://patchstorage.com/lilac-looper-stereo-example/)
- [Multi-track looping](https://patchstorage.com/lilac-looper-multi-track-example/)

Press the big button to toggle between record, play and overdub modes. A typical looping session might look like this:

```
Record → Play → Overdub → Play → Overdub → … → Stop → Play → … → Erase
```

### Interface & Controls

- **Toggle** is the big button that moves through the active stages of looping:
  - Record - create a first recording, setting the duration of the loop
  - Overdub - add new material into the loop
  - Play - continue listening to the loop without adding to it
- **Status lights** below the big button show when you're recording (red) and playing (green); The light blinks every time Looper reaches the starting point of the loop.
- **Stop** stops the loop. Pressing toggle while stopped will restart the loop from the beginning.
- **Erase** removes a recorded loop from memory irreversibly allowing you to record a new loop. Looper doesn't actively prevent "pops" or "clicks" when erasing the loop (in contrast to other mode changes which are smooth and quiet). You can avoid "pops" by stopping the loop before erasing it.
- Use the "**After first loop…**" option in the module's context menu to choose which mode is toggled after recording your first loop. The default setting is "Play". The "Overdub" setting will continue recording after recording your first loop

### Stereo and Multi-Track Recording

Although Looper works in mono by default, it's capable of recording up to 16 tracks at once thanks to VCV Rack's [polyphonic cables](https://vcvrack.com/manual/Polyphony). To record a multi-channel loop, connect a polyphonic cable to the input and press the big toggle button to start recording. Each channel on the polyphonic input will record on a dedicated internal channel and play back on the corresponding output channel. Use VCV's [Merge](https://library.vcvrack.com/Fundamental/Merge) and [Split](https://library.vcvrack.com/Fundamental/Split) modules to manage polyphony.

To record in **stereo**, simply use voices 1 & 2 of a polyphonic signal (as seen in the [stereo example](https://patchstorage.com/lilac-looper-stereo-example/)).

⚠️ Note that every input channel you plan to record must be connected before you start recording a new loop ([issue #3](https://github.com/grough/lilac-loop-vcv/issues/3)). Additional input channels connected after the first loop is recorded will be ignored until you erase and start a new recording.

### Input Monitoring

Looper doesn't mix its input signal with its output. If you want to hear live input along with a recorded loop, connect your live input and Looper's output into a mixer.

### Known Issues & Limitations

If you'd like to report a bug or are having trouble using the plugin, please submit an [issue](https://github.com/grough/lilac-loop-vcv/issues). Following are some known issues that may be solved in a future release:

- Number of polyphonic channels is constant per recording ([#3](https://github.com/grough/lilac-loop-vcv/issues/3))
- Erasing a loop during playback makes a "pop" or "click" sound
- Looper does not enforce a maximum loop length

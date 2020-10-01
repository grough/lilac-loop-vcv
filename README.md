This is a plugin for [VCV Rack](https://vcvrack.com/). As of version `1.0.0` it contains just one module, [Lilac Looper](#lilac-looper). If you find a bug or have a feature request, please submit an [issue](https://github.com/grough/club-soda-vcv/issues).

## Lilac Looper

This module supports a recording workflow similar to that of a [looper pedal](https://en.wikipedia.org/wiki/Live_looping). It can record, overdub and play a single or multi-track audio loop. The controls are as follows:

### Toggle

_Toggle_ is the main control that moves through the active stages of looping:

- Record - create an initial recording, setting the duration of the loop
- Overdub - mix new material into the loop
- Play - continue listening to the loop without adding to it

A typical looping session might look like this:

```
REC → PLAY → OVER → PLAY → OVER → … → STOP → PLAY → … → ERASE
```

Status lights below toggle input show the active mode (`REC` red, `OVER` yellow, `PLAY` green, `STOP` blue). The light blinks whenever the loop ends and begins again.

Use the _After Record_ switch to select which mode is toggled after recording an initial loop. The default switch position will **play** immediately after recording; the opposite switch position will **overdub** instead.

### Stop

_Stop_ will stop the loop during any mode in which it's not already stopped. Pressing _Toggle_ while stopped will restart the loop from the beginning.

### Erase

_Erase_ removes an existing loop from memory irreversibly, allowing you to record a new loop. Lilac does not actively prevent "pops" or "clicks" when erasing the loop (in contrast to other mode changes which are smooth and quiet). You can avoid "pops" by stopping the loop before erasing it.

### Stereo and Multi-track Recording

Although Lilac works in mono by default, it's capable of recording up to 16 tracks at once thanks to VCV Rack's [polyphonic cables](https://vcvrack.com/manual/Polyphony). To record a multi-track loop, connect a polyphonic cable to the input and toggle _Record_ mode. Each polyphony voice will record to a dedicated internal track and play back on the corresponding output voice. Use VCV's [Merge](https://library.vcvrack.com/Fundamental/Merge) and [Split](https://library.vcvrack.com/Fundamental/Split) modules to manage polyphony.

To record in **stereo**, simply use voices 1 & 2 on your polyphonic input/output signals.

### Monitoring

This module does not support input monitoring. If you want to hear live input along with the loop content, connect your live input and Lilac's output into a mixer.

<!--
### Known Issues

- There is no maximum loop length, so recording indefinitely will just eat memory
- Use sequential polyphonic voices to avoid unnecessary memory usage
- No signal smoothing on erase
-->

## Bugs

To bug please submit an [issue](https://github.com/grough/club-soda-vcv/issues).

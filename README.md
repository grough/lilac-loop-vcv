This is a plugin for [VCV Rack](https://vcvrack.com/). As of version `1.0.0` it contains just one module, [Lilac Looper](#lilac-looper). If you find a bug or need help, please submit an [issue](https://github.com/grough/club-soda-vcv/issues).

## Lilac Looper

This module implements features commonly found on a looper pedal. It lets you record, overdub and play a monophonic audio loop. The controls are as follows:

### Toggle

_Toggle_ is the main control that moves through the active stages of looping:

- Record - create an initial recording, setting the duration of the loop
- Overdub - mix new material into the loop
- Play - continue listening to the loop without adding to it

A typical looping session might look like this:

```
REC → PLAY → OVER → PLAY → OVER → … → STOP → PLAY → … → ERASE
```

Status lights below toggle input show the active mode (`REC` red, `OVER` yellow, `PLAY` green, `STOP` blue).

Use the _After Record_ switch to select which mode is toggled after recording an initial loop. The default switch position will **play** immediately after recording; the other switch position will **overdub** instead.

### Stop

_Stop_ will stop the loop during any mode in which it's not already stopped. Pressing _Toggle_ while stopped will restart the loop from the beginning.

### Erase

_Erase_ removes an existing loop from memory irreversibly, allowing you to record a new loop. As of version `1.0.0` _Lilac_ does not actively prevent "pops" or "clicks" when erasing the loop (in contrast to the other mode changes which are smooth and quiet). You can avoid "pops" by stopping the loop before erasing it.

### Monitoring

This module does not support input monitoring. If you want to hear live input along with the loop content, connect your live input and _Lilac_'s output into a mixer.

<!--
### Known Issues

- There is no maximum loop length, so recording indefinitely will just eat memory
- No signal smoothing on erase
-->

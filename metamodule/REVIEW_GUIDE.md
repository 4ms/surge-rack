# Native Surge port: design and review guide

This port runs Surge sound engines behind MetaModule processors. It does not
instantiate the desktop Rack modules, except for two small Unison wrappers
that use the SDK's Rack interface to participate in firmware expander linking.
The desktop Hub translates Rack patches into this native module vocabulary.

## Read in this order

| Concern | Starting point | What to verify |
| --- | --- | --- |
| Upstream adaptations | [runtime source map](runtime/UPSTREAM_MAP.md) | Imported algorithms versus host, storage, reset and numerical changes |
| Engine boundary | [engine.h](dsp/engine.h), [engine.cpp](dsp/engine.cpp) | Normalized controls, input routing, block processing and output stage |
| SDK boundary | [oscillator_processor.h](oscillator_processor.h), [utility_processor.h](utility_processor.h) | Channel counts, mono broadcasting, bypass and state callbacks |
| Representative effect | [Chorus processor](chorus_processor.h), [Chorus DSP](dsp/chorus_dsp.cpp), [Chorus panel](chorus_panel.h) | One control's journey from panel to engine; native versus imported behavior |
| Panel and registration | [oscillator_panel.h](oscillator_panel.h), [plugin.cpp](plugin.cpp) | Stable IDs, stepped controls and model registration |
| Modulation helpers | [modulation_processor.h](modulation_processor.h), [modulation_panels.h](modulation_panels.h) | Depth scaling, polyphonic summation and destination labels |
| Tables and expanders | [table_oscillator.h](table_oscillator.h), [unison_processor.h](unison_processor.h) | Ownership across callbacks and bank/expander coordination |
| Build and evidence | [CMakeLists.txt](CMakeLists.txt), [validation](VALIDATION.md) | Dependencies, package contents and the limits of existing evidence |

The commit sequence introduces upstream adaptations, the DSP boundary, an
oscillator plugin, control/filter/expander modules, and the remaining effects.
The first two commits provide source dependencies; the oscillator commit first
activates the native plugin target. Later module commits extend registration
and metadata together. These intermediate configurations have received static
inspection, not separate builds.

## Boundaries and stable coordinates

`runtime/` contains extracted or adapted upstream production sources. `dsp/`
supplies the embedded host, storage and per-module engine interfaces. Processors
translate SDK calls into those interfaces; panels describe the controls and
jacks. CMake compiles DSP as C++17 and SDK integration as C++23.

Parameter and jack indices are serialized patch coordinates. Reordering a
panel's elements does not authorize renumbering its parameters or jacks.
`plugin.json`, `plugin-mm.json`, processor IDs and the Hub's conversion tables
must agree. An incompatible ID or state change needs an explicit migration.

SDK parameters are normalized to 0..1. Engine adapters convert them to the
original Surge ranges. Audio and CV at the processor boundary are volts;
there is no universal CV coefficient across all controls. For example, Chorus
uses 0.2 normalized units/V for ordinary controls and 12/130 units/V for its
cutoff controls. See the Hub guide for the corresponding depth conversion.

Oscillator parameters 0..9 are pitch, seven engine controls, drift and level.
Model-specific options start at 10. Panel input coordinates are:

| Input | Meaning |
| --- | --- |
| 0, 1 | Pitch and reset/trigger |
| 2..8 | Seven direct engine-control CVs |
| 9, 10 | Drift and level CV |
| 11 | Additional pitch CV for standard/Twist/table oscillators; audio for Alias/String |
| 12 | Additional pitch CV for Alias/String |

`SurgeOscillator::update()` translates these panel IDs to the engine's separate
input layout. Keep the translation explicit: engine modulation slots and panel
jacks are not interchangeable.

Most native processors use four voices. `UtilityProcessor::input()` broadcasts
a mono cable by default; callers disable it for inputs that follow per-lane
Rack semantics. Oscillators derive their voice count from pitch/reset inputs.
Effects choose their Sum/Poly behavior in their wrappers. Unison distributes
up to 16 voice slots across four banks; that is not a general 16-channel SDK
processor implementation.

## Native operation and imported Rack patches

Native panels expose fixed CV jacks so modulation can be patched directly.
Rack's four assignable modulation inputs can each affect several controls,
with a different signed depth for every destination. The Hub materializes
those assignments as visible attenuverters or matrix helpers and cables.
Depth knobs remain editable after transfer. Only needed routes are connected;
helper panel layouts themselves are static.

The matrix has eight output offsets (parameters 0..7), 32 signed depths
(8..39, destination-major), a minimum channel count (40), and eight selector
curves (41..48). At depth parameter `p`, its linear route contributes
`2 * (2*p - 1) * input_voltage`. A neutral offset or depth is 0.5.
Selector curves run after summation to preserve Alias and Unison selection
thresholds. Destination-specific variants change labels/groups, not this DSP.

Imported state markers select Rack connection, clock and initialization
behavior where it differs from the native defaults. They are compatibility
contracts, not UI preferences. For example, `surge-fx-clock-rack-v1` selects
the Chorus Rack clock/history path; resetting to empty state restores the
native defaults. Reverb's newer marker preserves its original voice history.
Both the Hub writer and native reader must change together when a contract
changes. [Known deviations](UPSTREAM_ISSUES.md) remain separate from these
compatibility mechanisms.

## Ownership and timing

Processors own their engine instances. DSP adapters buffer samples into the
eight-sample Surge block size, so block boundaries and reset order matter.
Review imported lifecycle paths as well as steady-state equations.

Table loading has a separate ownership protocol. GUI callbacks construct a
table and publish a `Packet` through `pending`. The audio callback adopts it
and puts the previous packet on `retired`; the GUI reclaims retired packets.
The retired packet retains the old table handle so changing voices does not
free its backing storage in the audio callback. Review `submit()`, `update()`,
`collect()` and module destruction together, including the SDK's callback
lifetime assumptions.

Unison uses the SDK Rack adapter because firmware expander discovery operates
on `rack::engine::Module`. Its routing dimensions are published atomically
because helper and expander can run on different cores. The Hub expands
supported 16-voice graphs into four-lane copies and duplicates mappings/state;
unsupported graph shapes fail export. CPU, memory and extra helper scheduling
costs still require hardware measurement.

## Maintenance boundaries

Update the pinned source and its adaptations together; consult the source map
before copying a new upstream implementation. Keep algorithm changes separate
from platform integration. Existing numerical and behavioral deviations are
listed in [UPSTREAM_ISSUES.md](UPSTREAM_ISSUES.md), including unresolved ones.
The map is provenance, not a claim that every adaptation is behavior-neutral.

The Base64 helper in `dsp/table_encoding.hh` is also used by the desktop Hub
as `src/mapping/table_encoding.hh`. The implementations currently match apart
from formatting. Changes to the table encoding must update both copies and
their readers; neither build should depend on a sibling working directory.

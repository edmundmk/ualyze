# Ualyze

Ualyze is a library which performs the following Unicode analyses:

  * Line breaking algorithm [UAX #14](https://unicode.org/reports/tr14/).
  * Grapheme cluster breaking algorithm [UAX #29](https://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries).
  * Bracket-aware script classification [UAX #24](https://unicode.org/reports/tr24/#Classification_by_Script).
  * Bidirectional algorithm [UAX #9](https://unicode.org/reports/tr9/).

It depends on a slightly [customised version of UCDN](https://github.com/edmundmk/ucdn)
(forked from the original [here](https://github.com/grigorig/ucdn)) to query
Unicode properties.

Ualyze is designed to be used as part of a text layout system.

For example, when using [FreeType](https://freetype.org) (to render glyphs)
and [Harfbuzz](http://harfbuzz.org) (to shape glyph runs), your application
must provide correct information about the script and bidi level of a span of
text.  This library can be used to generate this information.

Also, caret positioning requires information about grapheme clusters, and word
wrap requires identification of potential break points.


## API

### Buffer

Analysis is performed on text stored in a `ual_buffer`.  This is a refcounted
object.

    ual_buffer* ub = ual_buffer_create();
    ub = ual_buffer_retain( ub );
    ual_buffer_release( ub );
    ual_buffer_release( ub );


### Text

Text is UTF-16, provided as `char16_t`.  UTF-16 is the most common encoding
used by UI and font systems.

    ual_buffer_clear( ub );
    ual_buffer_append( ub, { u"Hello, World!", 13 } );

Invalid UTF-16 sequences are treated by the library as if they were replaced
by U+FFFD REPLACEMENT CHARACTER.


### Paragraphs

Text is split into paragraphs by hard line breaks.  Each paragraph is
identified and visited in sequence.

    ual_paragraph p;
    while ( ual_next_paragraph( ub, &p ) )
    {
        // Analyze this paragraph.
    }

The `ual_char` array and indexes produced by analysis passes are relative to
the current paragraph, not the entire text.


## Analysis

### Line and Cluster Breaking

Call `ual_break_analyze` to perform line and cluster breaking.  The results of
break analysis are stored in the `ual_char` buffer.

  * The `UAL_BREAK_CLUSTER` flag is set on the encoding unit which starts each
    grapheme cluster.

  * The `UAL_BREAK_LINE` flag is set on the encoding unit which would start
    the next line, if the break opportunity is taken.

  * The `UAL_BREAK_SPACES` flag is set at the start of the first space
    character in a run of space characters immediately preceding a break
    opportunity.  These runs of spaces usually do not contribute to the layout
    width of a line.

Break points can be extremely frequent (especially grapheme cluster breaks).


### Script Analysis

A paragraph is broken into spans containing characters with the same script
classification.  The information is reported using an iterator-style interface.

    ual_script_span span;
    ual_script_spans_begin( ub );
    while ( ual_script_spans_next( ub, &span ) )
    {
        // Process this script span.
    }
    ual_script_spans_end( ub );


### Bidi Algorithm

The bidi algorithm proceeds in two stages.  First, call `ual_bidi_analyze` to
identify isolating run sequences and resolve the final bidi class of each
character in the paragraph.

Then, bidi runs are generated using an iterator-style interface.

    unsigned paragraph_level = ual_bidi_analyze( ub );

    ual_bidi_run run;
    ual_bidi_runs_begin( ub );
    while ( ual_bidi_runs_next( ub, &run ) )
    {
        // Process this bidi run.
    }
    ual_bidi_runs.end( ub );

The `ual_bidi_paragraph_level` function does only enough processing to
determine the paragraph level (either 0 left or 1 right) for the current
paragraph.  This can be useful when using certain text layout APIs which
perform bidi processing without providing this information explicitly.


## Shared Resources

Certain resources owned by the `ual_buffer` are shared between analysis passes.

The `bc` member of the `ual_char` structure has two different uses:

  * After `ual_break_analyze`, this member stores break flags.
  * After `ual_bidi_analyze`, this member stores the resolved bidi class.

Generation of bidi runs can only be performed with resolved bidi classes.

There is also an internal shared stack, which is in use:

  * When iterating through script spans.
  * During the `ual_bidi_analyze` function.

Therefore, `ual_bidi_analyze` should be called before beginning iteration of
script spans.  Once bidi analysis has completed, bidi runs and script spans can
be iterated at the same time - bidi processing only requires the stack during
the analysis pass.


## Status

Ualyze is based on code I've been working on for a while, but hasn't been used
in anger yet.  I make no promises about its stability or correctness.


## License

Copyright © 2019 Edmund Kapusniak.  Licensed under the MIT License. See the
LICENSE file in the project root for full license information.


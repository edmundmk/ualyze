# Ualyze

Ualyze is a library which performs the following Unicode analyses:

  * Line breaking algorithm [UAX #14](https://unicode.org/reports/tr14/).
  * Grapheme cluster breaking algorithm [UAX #29](https://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries).
  * Bracket-aware script classification [UAX #24](https://unicode.org/reports/tr24/#Classification_by_Script).
  * Bidirectional algorithm [UAX #9](https://unicode.org/reports/tr9/).

Ualyze is designed to be used as part of a text layout system.

For example, when using [FreeType](https://freetype.org) (to render glyphs)
and [Harfbuzz](http://harfbuzz.org) (to shape glyph runs), your application
must provide correct information about the script and bidi level of a span of
text.  This library can be used to generate this information.

Also, caret positioning requires information about grapheme clusters, and word
wrap requires identification of potential break points.


## API

### Text

Analysis data is stored in a `ual_buffer`.  This is a refcounted object.

    ual_buffer* ub = ual_buffer_create();
    ub = ual_buffer_retain( ub );
    ual_buffer_release( ub );
    ual_buffer_release( ub );

Text is UTF-16, provided by the client as a `char16_t` string.  This string
must be kept alive during analysis.

The string is split into paragraphs by hard line breaks.  Each paragraph is
analysed independently.  Each call to `ual_analyze_paragraph` identifies the
first paragraph in the string, and sets up analysis for the characters in
that paragraph.

    while ( size )
    {
        /* identify paragraph. */
        size_t length = ual_analyze_paragraph( ub, text, size );

        /* analyze this paragraph. */

        /* move to next paragraph. */
        assert( length <= size );
        text += length;
        size -= length;
    }

Invalid UTF-16 sequences are treated by the library as if they were replaced
by U+FFFD REPLACEMENT CHARACTER.

The internal `ual_char` buffer has one entry for each encoding unit in the
currently analyzed paragraph.


### Line and Cluster Breaking

Call `ual_analyze_breaks` to perform line and cluster breaking.  The results of
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
        /* process this script span. */
    }
    ual_script_spans_end( ub );


### Bidi Algorithm

The bidi algorithm proceeds in two stages.  First, call `ual_bidi_analyze` to
identify isolating run sequences and resolve the final bidi class of each
character in the paragraph.

Then, bidi runs are generated using an iterator-style interface.

    unsigned paragraph_level = ual_analyze_bidi( ub, UAL_FROM_TEXT );

    ual_bidi_run run;
    ual_bidi_runs_begin( ub );
    while ( ual_bidi_runs_next( ub, &run ) )
    {
        /* process this bidi run. */
    }
    ual_bidi_runs_end( ub );


## Shared Resources

Certain resources owned by the `ual_buffer` are shared between analysis passes.

The `bc` member of the `ual_char` structure has two different uses:

  * After `ual_analyze_breaks`, this member stores break flags.
  * After `ual_analyze_bidi`, this member stores the resolved bidi class.

Generation of bidi runs can only be performed with resolved bidi classes.

There is also an internal shared stack, which is in use:

  * When iterating through script spans.
  * During the `ual_analyze_bidi` function.

Therefore, `ual_analyze_bidi` should be called before beginning iteration of
script spans.  Once bidi analysis has completed, bidi runs and script spans can
be iterated at the same time - bidi processing only requires the stack during
the analysis pass.


## Status

Ualyze is based on code I've been working on for a while, but hasn't been used
in anger yet.  I make no promises about its stability or correctness.


## Acknowledgements

Previous versions of this project used [UCDN](https://github.com/grigorig/ucdn)
by Grigori Goronzy to query Unicode character properties.  It has been updated
to use an internal set of Unicode property values generated from the Unicode
Character Database.


## License

Licensed under the ISC License.  See the LICENSE file in the project root for
full license information.


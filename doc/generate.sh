
StrfVersion=v0.12.2

mkdir -p out
cp syntax.svg out/syntax.svg

asciidoctor -v introduction.adoc -o  out/introduction.html
asciidoctor -v quick_reference.adoc -o  out/quick_reference.html
asciidoctor -v locale_hpp.adoc -o out/locale_hpp.html

asciidoctor -v  --attribute=strf-version=$StrfVersion \
            benchmarks.adoc -o  out/benchmarks.html
asciidoctor -v --attribute=strf-version=$StrfVersion \
            cuda.adoc -o out/cuda.html
asciidoctor -v --attribute=strf-version=$StrfVersion \
            howto_add_destination.adoc -o out/howto_add_destination.html
asciidoctor -v --attribute=strf-version=$StrfVersion \
            howto_add_printable_types.adoc -o out/howto_add_printable_types.html

asciidoctor -v strf_hpp.adoc -o - \
    | sed 's/20em/34em/g' \
    | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' \
    > out/strf_hpp.html

asciidoctor -v outbuff_hpp.adoc -o - \
    | sed 's/20em/25em/g' \
    | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' \
    > out/outbuff_hpp.html

asciidoctor -v to_cfile_hpp.adoc -o - \
    | sed 's/20em/25em/g' \
    | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' \
    > out/to_cfile_hpp.html

asciidoctor -v to_streambuf_hpp.adoc -o - \
    | sed 's/20em/25em/g' \
    | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' \
    > out/to_streambuf_hpp.html

asciidoctor -v to_string_hpp.adoc -o - \
    | sed 's/20em/25em/g' \
    | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' \
    > out/to_string_hpp.html


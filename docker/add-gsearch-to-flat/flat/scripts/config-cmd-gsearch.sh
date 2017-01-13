#!/bin/bash

# Welcome info
show_welcome() {
	cat << EOF
FLAT GSearch Component Metadata facet mapping!
<><><><><><><><><><><><><><><><><><><><><><><>
EOF
}

# Usage info
show_help() {
	show_welcome
	cat << EOF
Usage: ${0##*/} [-hv] [-c CLFACET] [-g GSMAP] [-t GSTRANS] [-s GSSCHEMA] [-d DCTRANS]  [-p CACHEDIR] [-r REGURL] [-i INDIR] [-x EXT] [-o OUTPREF]
Determine the mappings for the CMD records in INDIR and write them to a set of output files with OUTPREF.

    -h          display this help and exit
    -c CLFACET  location of the CLARIN facet mapping (URL)
                (default: https://raw.githubusercontent.com/clarin-eric/VLO/master/vlo-commons/src/main/resources/facetConcepts.xml)
    -g GSMAP    location of the GSearch facet mapping template
                (default: ./gsearch-mapping-template.xml)
    -t GSTRANS  location of the GSearch transformer
                (default: ./fedoragsearch-2.6/fedoragsearch/FgsConfig/generated_gsearch_config_for_islandora/fgsconfigFinal/index/FgsIndex/foxmlToSolr.xslt)
    -s GSSCHEMA location of the GSearch Solr schema
                (default: ./fedoragsearch-2.6/fedoragsearch/FgsConfig/generated_gsearch_config_for_islandora/fgsconfigFinal/index/FgsIndex/conf/schema-4.2.0-for-fgs-2.6.xml)
    -d DCTRANS  location of the CMD2DC transformer template
                (default: ./cmd2dc-template.xsl)
    -p CACHEDIR location of the CMD profile cache
                (default: ./.profile-cache)
    -r REGURL   URL of profiles endpoint of the CLARIN Component Registry
                (default: http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles)
    -i INDIR    examine the records in this directory
                (default: .)
    -x EXT      extension of the records in the input directory
                (default: xml)
    -o OUTPREF  put the resulting mapping, transformer and schema in files with this prefix
                (default: ./lat-gsearch)
    -v          be verbose.
EOF
}

# Initialize our own variables:
clarin_fc="https://raw.githubusercontent.com/clarin-eric/VLO/master/vlo-commons/src/main/resources/facetConcepts.xml"
gsearch_fc="./gsearch-mapping-template.xml"
gsearch_xsl="./fedoragsearch-2.6/fedoragsearch/FgsConfig/generated_gsearch_config_for_islandora/fgsconfigFinal/index/FgsIndex/foxmlToSolr.xslt"
gsearch_solr="./fedoragsearch-2.6/fedoragsearch/FgsConfig/generated_gsearch_config_for_islandora/fgsconfigFinal/index/FgsIndex/conf/schema-4.2.0-for-fgs-2.6.xml"
cmd2dc_xsl="./cmd2dc-template.xsl"
profile_cache="./.profile-cache"
registry="http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles"
input_dir="."
rec_ext="xml"
output_prefix="./lat-gsearch"
verbose=0

OPTIND=1 # Reset
while getopts "h?c:g:t:s:p:r:i:x:o:v" opt; do
	case "$opt" in
		h|\?)
			show_help
			exit 0
			;;
		v)
			verbose=1
			;;
		c)
			clarin_fc=$OPTARG
			;;
		g)
			gsearch_fc=$OPTARG
			;;
		t)
			gsearch_xsl=$OPTARG
			;;
		s)
			gsearch_solr=$OPTARG
			;;
		d)
			cmd2dc_xsl=$OPTARG
			;;
		p)
			profile_cache=$OPTARG
			;;
		r)
			registry=$OPTARG
			;;
		i)
			input_dir=$OPTARG
			;;
		x)
			rec_ext=$OPTARG
			;;
		o)
			output_prefix=$OPTARG
			;;
		'?')
			show_help >&2
			exit 1
			;;
	esac
done
shift "$((OPTIND-1))" # Shift off the options and optional --.

if [ $verbose -ne 0 ]; then
	show_welcome
	echo "Run: `date`"
	echo "Input directory: $input_dir"
fi

profiles="`./findProfiles.sh -e "$rec_ext" $input_dir`"

if [ $verbose -ne 0 ]; then
	echo "Number of profiles: `echo $profiles | wc -w`"
	echo "Profiles: `for p in $profiles; do echo -n "$p ";done`"
fi

if [ $verbose -ne 0 ]; then
	echo "Profile cache: $profile_cache"
fi

if [ ! -d $profile_cache ]; then
	mkdir -p $profile_cache
	if [ $verbose -ne 0 ]; then
		echo "Created profile cache directory"
	fi
else
	rm -rf $profile_cache/*
	if [ $verbose -ne 0 ]; then
		echo "Cleaned profile cache directory"
	fi
fi

if [ $verbose -ne 0 ]; then
	echo "Profile registry: $registry"
fi

for profile in $profiles; do
	PROF="`echo $profile | sed -e 's|[^a-zA-Z0-9]|_|g'`"
	if [ ! -f $profile_cache/$PROF.xml ]; then
		curl -s -o "$profile_cache/$PROF.xml" "$registry/$profile/xml"
                if [ $? -ne 0 ]; then
                        if [ -f $profile_cache/$PROF.xml ]; then
                                rm $profile_cache/$PROF.xml
                        fi
                        echo "Failed to fetch profile: $profile from $registry/$profile/xml to $profile_cache/$PROF.xml"
                else
                        grep 'Profile not found:' $profile_cache/$PROF.xml > /dev/null
                        if [ $? -eq 0 ]; then
                                rm $profile_cache/$PROF.xml
                                echo "Profile doesn't exist: $profile from $registry/$profile/xml to $profile_cache/$PROF.xml"
                        else
                                if [ $verbose -ne 0 ]; then
                                        echo "Fetched profile: $profile from $registry/$profile/xml to $profile_cache/$PROF.xml"
                                fi
                        fi
                fi
	fi
done

if [ $verbose -ne 0 ]; then
	echo "CLARIN facet mapping: $clarin_fc"
	echo "GSearch facet mapping: $gsearch_fc"
fi

# get the directory the script is located in
#my_dir="${0%%/*}"
my_dir="$(dirname "$(readlink -f "$0")")"
# relative to this directory, invoke the xsl transformation
./xsl2.sh -xsl:${my_dir}/createMapping.xsl -s:$gsearch_fc clarin_fc=$clarin_fc profile_cache=$profile_cache > ${output_prefix}-mapping.xml

if [ $verbose -ne 0 ]; then
	echo "Output mapping file: ${output_prefix}-mapping.xml"
	#echo "Output mapping:"
	#cat ${output_prefix}-mapping.xml
fi

if [ $verbose -ne 0 ]; then
	echo "GSearch transformer: $gsearch_xsl"
fi

./xsl2.sh -xsl:${my_dir}/createTransformer.xsl -s:$gsearch_xsl mapping-location=${output_prefix}-mapping.xml > ${output_prefix}-transformer.xsl

if [ $verbose -ne 0 ]; then
	echo "Output GSearch transformer file: ${output_prefix}-transformer.xsl"
	#echo "Output transformer:"
	#cat ${output_prefix}-transformer.xsl
fi

if [ $verbose -ne 0 ]; then
	echo "GSearch SOLR schema: $gsearch_solr"
fi

./xsl2.sh -xsl:${my_dir}/createSchema.xsl -s:$gsearch_solr mapping-location=${output_prefix}-mapping.xml > ${output_prefix}-schema.xml

if [ $verbose -ne 0 ]; then
	echo "Output SOLR schema file: ${output_prefix}-schema.xml"
	#echo "Output SOLR schema:"
	#cat ${output_prefix}-schema.xml
fi

if [ -n "$cmd2dc_xsl" ] && [ -f "$cmd2dc_xsl" ]; then

    if [ $verbose -ne 0 ]; then
    	echo "CMD2DC transformer template: $cmd2dc_xsl"
    fi
    
    ./xsl2.sh -xsl:${my_dir}/createDublinCore.xsl -s:$cmd2dc_xsl mapping-location=${output_prefix}-mapping.xml > ${output_prefix}-cmd2dc.xsl
    
    if [ $verbose -ne 0 ]; then
    	echo "Output CMD2DC transformer file: ${output_prefix}-cmd2dc.xsl"
    	#echo "Output transformer:"
    	#cat ${output_prefix}-transformer.xsl
    fi

fi

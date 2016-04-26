#!/bin/bash
if [ $# -ne 2 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym and bag_id of the project"
  exit 1
else
  user=$1
  bag_id=$2
bag_dir=$3
fi

export JAVA_HOME=/opt/jdk1.8.0_72
PATH=$JAVA_HOME:$PATH

export FEDORA_HOME="/var/www/fedora"

base_dir=/app/flat/


target_dir="$bag_dir"/"$bag_id"

for d in fox fox-error
do
  if [ -d $target_dir/$d ];then
    rm -rf $target_dir/$d
  fi
done

java -Xmx4096m -jar $base_dir/lib/lat2fox.jar -d=$base_dir/cmd2dc.xsl $target_dir

$FEDORA_HOME/client/bin/fedora-batch-ingest.sh $target_dir/fox $base_dir/log xml info:fedora/fedora-system:FOXML-1.1 localhost:8443 fedoraAdmin fedora https fedora >$target_dir/fedora_batch.log

cat $target_dir/fedora_batch.log | grep ^"ingest succeeded for:"

echo "nIngested: $(cat $target_dir/fedora_batch.log | grep ^"ingest succeeded for:" |  wc -l)"
exit 0



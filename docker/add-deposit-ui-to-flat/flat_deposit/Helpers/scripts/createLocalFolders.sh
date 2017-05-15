#!/bin/bash

if [ $# -ne 1 ];then
  echo "Not enough parameters specified"
  echo "Please provide (1) base directory"
  exit 1

else
    base=$1
fi

if [ ! -d "$base" ]; then

  echo "Specified base directory does not exist"
  exit 1
fi

for departments in lac lag ladd nbl pol nvc tg; do

 mkdir -p $base/$departments/general
 mkdir -p $base/$departments/shared

  for project in study_1 study_2 study_3; do

    project_dir=$base/$departments/workspaces/${departments}_${project}
    archive_deposit_dir=$project_dir/archive_deposit

    for sub in analysis archive archive_deposit primary_data working_data; do

      mkdir -p $project_dir/$sub

    done;

    f=$(mktemp);
    echo "This is the data for project $project of department $departments" >$f
    enscript $f -o - | ps2pdf - "$archive_deposit_dir/data_${departments}_${project}.pdf"

    chmod -R 0755 $(dirname $project_dir)
    chmod -R 0700 $archive_deposit_dir
    chown -R www-data:www-data $archive_deposit_dir

  done;

done;


echo 'script executed successfully';
exit 0;

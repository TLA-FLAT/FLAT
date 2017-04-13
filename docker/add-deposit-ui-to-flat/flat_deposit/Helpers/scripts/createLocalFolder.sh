#!/bin/bash

base=/app/flat/deposit/local/
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
    enscript $f -o - | ps2pdf - $archive_deposit_dir/data.pdf

    chown -R www-data:www-data $archive_deposit_dir
    chmod -R 0600 $archive_deposit_dir


  done;
done;


exit 1;

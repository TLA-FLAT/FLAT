<?php


/*
 * Custom page callback for admin/reports/node-list
 */
function flat_deposit_ui_list_uploads(){

    //define variants
    $node_types = node_type_get_types();
    #krumo($node_types);
    $stati = array(1); //0 = unpublished, 1 = published


    //loop through variants
    foreach($node_types as $type) {
        #krumo($type);
        $typename = $type->type;
        foreach($stati as $status) {
            #krumo($status);

            //SELECT COUNT FROM node WHERE type = $typename AND status = $status
            $query = db_select('node', 'n')
                ->condition('type', $typename)
                ->condition('status', $status);
            $query->addExpression('COUNT (nid)', 'node_count');
            $results = $query->execute();
            $data = $results->fetchObject();
            #krumo($data);

            //assign vars for printing
            if ($status == 0) {
                $status = 'unpublished';
            }
            else {
                $status = 'published';
            }

            //assemble strings for output
            $items[] = t('The %type content type has %num %status nodes.', array('%type'=>$typename, '%num'=>$data->node_count, '%status'=>$status ));

        }  //end status loop
    }  //end node type loop

    #krumo($items);

    $content['node-list'] = array(
        '#items' => $items,
        '#theme' => 'item_list',
    );

    return render($content);

}




<?php
  $output = shell_exec('./tp2 '.$_GET["ip"].' '.$_GET["port"].' 2');
  $data = json_decode($output, TRUE);
  $results = array( "sEcho" => 1,
                    "iTotalRecords" => count($data),
                    "iTotalDisplayRecords" => count($data),
                    "aaData"=>$data);
  echo json_encode($results);
?>

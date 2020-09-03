<?php
  if( $_REQUEST["ip"] || $_REQUEST["port"] ) {
    $output = shell_exec('./tp2 '.$_POST["ip"].' '.$_POST["port"].' 2');

    $data = json_decode($output, TRUE);

    $npids = sizeof($data);

    $ramPercentage = 0;
    $cpuPercentage = 0;

    foreach($data as $d) {
       $ramPercentage += $d["memory"];
       $cpuPercentage += $d["cpu"];
    }

    $cpu = array(); $memory = array();
    for ($i=0; $i < 10; $i++) {
      $cpu[$i] = array("label" => $data[$i]["name"] , "y" => $data[$i]["cpu"]/100);
      $memory[$i] = array("label" => $data[$i]["name"] ,"y" => $data[$i]["memory"]/100);
    }
  }else{
    header('Location: login.php');
    exit;
  }
 ?>
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <meta name="description" content="Gestão de Processos - SNMP">
    <meta name="author" content="Hugo Machado">

    <title>Gestão de Processos - SNMP</title>
    <!-- Custom fonts for this template -->
    <link href="vendor/fontawesome-free/css/all.min.css" rel="stylesheet" type="text/css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
    <link href="https://fonts.googleapis.com/css?family=Nunito:200,200i,300,300i,400,400i,600,600i,700,700i,800,800i,900,900i" rel="stylesheet">
    <!-- Custom styles for this template -->
    <link href="css/sb-admin-2.min.css" rel="stylesheet">
    <!-- Custom styles for this page -->
    <link href="vendor/datatables/dataTables.bootstrap4.min.css" rel="stylesheet">
  </head>
  <body id="page-top">
    <!-- Page Wrapper -->
    <div id="wrapper">
      <!-- Content Wrapper -->
      <div id="content-wrapper" class="d-flex flex-column">
        <!-- Main Content -->
        <div id="content">
          <!-- Topbar -->
          <nav class="navbar navbar-expand navbar-light bg-white topbar mb-4 static-top shadow">
            <!-- Topbar Navbar -->
            <ul class="navbar-nav ml-auto">
              <!-- Nav Item - Alerts -->
              <li class="nav-item dropdown no-arrow mx-1">
                <a class="nav-link dropdown-toggle" href="#" id="alertsDropdown" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                  <i class="fas fa-list fa-fw"></i>
                  <!-- Counter - Alerts -->
                  <span class="badge badge-danger badge-counter">3+</span>
                </a>
                <!-- Dropdown - Alerts -->
                <div class="dropdown-list dropdown-menu dropdown-menu-right shadow animated--grow-in" aria-labelledby="alertsDropdown">
                  <h6 class="dropdown-header">
                    Historico
                  </h6>
                  <?php
                      $subject = shell_exec("ls history/");
                      preg_match_all('/\d+/', $subject, $matches);
                      foreach ($matches[0] as $m) {
                        echo "  <a class=\"dropdown-item d-flex align-items-center\" href=\"#\" data-toggle=\"modal\" data-target=\"#history".$m."\">
                                  <div class=\"mr-3\">
                                    <div class=\"icon-circle bg-primary\">
                                      <i class=\"fas fa-file-alt text-white\"></i>
                                    </div>
                                  </div>
                                  <div>
                                    <div class=\"small text-gray-800 font-weight-bold\">".gmdate("d/M/Y H:i:s", $m)."</div>
                                  </div>
                                </a>";
                      }
                  ?>
                </div>
              </li>

              <!-- Nav Item - User Information -->
              <li class="nav-item dropdown no-arrow">
                <a class="nav-link dropdown-toggle" href="#" id="userDropdown" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                  <span class="mr-2 d-none d-inline text-gray-800">Bruno Dias</span>
                  <img class="img-profile rounded-circle" src="https://media.gettyimages.com/vectors/new-world-order-symbol-with-allseeing-eye-of-providence-novus-ordo-vector-id1006420892?b=1&k=6&m=1006420892&s=170x170&h=ZGa3zY92wNPnDxepkTH5dXkx9hslk3sHfzab0QXgBPQ=">
                </a>
                <!-- Dropdown - User Information -->
                <div class="dropdown-menu dropdown-menu-right shadow animated--grow-in" aria-labelledby="userDropdown">
                  <a class="dropdown-item" href="#" data-toggle="modal" data-target="#logoutModal">
                    <i class="fas fa-sign-out-alt fa-sm fa-fw mr-2 text-gray-400"></i>
                    Sair
                  </a>
                </div>
              </li>

            </ul>
          </nav>
          <!-- End of Topbar -->
          <!-- Begin Page Content -->
          <div class="container-fluid">
            <!-- Content Row -->
            <div class="row">
              <!-- Detalhes Card Example -->
              <div class="col-xl-4 col-md-4 mb-4">
                <div class="card border-left-primary shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-primary text-uppercase mb-1">Detalhes</div>
                        <div class="h6 mb-0 font-weight-bold text-gray-800">IP: <?php echo $_POST["ip"]; ?></div>
                        <div class="h6 mb-0 font-weight-bold text-gray-800">Porta: <?php echo $_POST["port"]; ?></div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-laptop fa-2x text-gray-300"></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>

              <!-- CPU Card Example -->
              <div class="col-xl-4 col-md-4 mb-4">
                <div class="card border-left-info shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-info text-uppercase mb-1">CPU utilizada</div>
                        <div class="row no-gutters align-items-center">
                          <div class="col-auto">
                            <div class="h5 mb-0 mr-3 font-weight-bold text-gray-800"><?php echo $cpuPercentage ?>%</div>
                          </div>
                          <div class="col">
                            <div class="progress progress-sm mr-2">
                              <div class="progress-bar bg-info" role="progressbar" style="width: <?php echo $cpuPercentage ?>%" aria-valuenow="<?php echo $cpuPercentage ?>" aria-valuemin="0" aria-valuemax="100"></div>
                            </div>
                          </div>
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-clipboard-list fa-2x text-gray-300"></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>

              <div class="col-xl-4 col-md-4 mb-4">
                <div class="card border-left-info shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-info text-uppercase mb-1">RAM utilizada</div>
                        <div class="row no-gutters align-items-center">
                          <div class="col-auto">
                            <div class="h5 mb-0 mr-3 font-weight-bold text-gray-800"><?php echo $ramPercentage ?>%</div>
                          </div>
                          <div class="col">
                            <div class="progress progress-sm mr-2">
                              <div class="progress-bar bg-info" role="progressbar" style="width: <?php echo $ramPercentage ?>%" aria-valuenow="<?php echo $ramPercentage ?>" aria-valuemin="0" aria-valuemax="100"></div>
                            </div>
                          </div>
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-clipboard-list fa-2x text-gray-300"></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <!-- Bar Chart -->
            <div class="card shadow mb-4">
              <div class="card-header py-3">
                <h6 class="m-0 font-weight-bold text-primary">Gráfico de barras com os 10 Processos a utilizar mais RAM</h6>
              </div>
              <div class="card-body">
                <div class="chart-bar">
                  <div id="chartContainer" style="height: 370px; width: 100%;"></div>
                </div>
              </div>
            </div>
            <!-- DataTales Example -->
            <div class="card shadow mb-4">
              <div class="card-header py-3">
                <h6 class="m-0 font-weight-bold text-primary">Tabela do Processos ativos</h6>
              </div>
              <div class="card-body">
                <div class="table-responsive">
                  <table class="table table-bordered" id="empTable" width="100%" cellspacing="0">
                    <thead>
                        <tr>
                            <th>Identificação do Processo</th>
                            <th>Utilização de CPU em %</th>
                            <th>Memória RAM em %</th>
                            <th>Nome da Aplicação</th>
                        </tr>
                    </thead>
                    <tfoot>
                        <tr>
                            <th>Identificação do Processo</th>
                            <th>Utilização de CPU em %</th>
                            <th>Memória RAM em %</th>
                            <th>Nome da Aplicação</th>
                        </tr>
                    </tfoot>
                  </table>
                </div>
              </div>
            </div>
          </div>
          <!-- /.container-fluid -->
        </div>
        <!-- End of Main Content -->

        <!-- Footer -->
        <footer class="sticky-footer bg-white">
          <div class="container my-auto">
            <div class="copyright text-center my-auto">
              <span>Copyright &copy; Gestão de Redes 2019</span>
              <span>Universidade do Minho</span>
            </div>
          </div>
        </footer>
        <!-- End of Footer -->
      </div>
      <!-- End of Content Wrapper -->
    </div>
    <!-- End of Page Wrapper -->

    <!-- Scroll to Top Button-->
    <a class="scroll-to-top rounded" href="#page-top">
      <i class="fas fa-angle-up"></i>
    </a>

    <!-- History Modal-->
    <?php 
        foreach ($matches[0] as $m) {
          echo "
          <div class=\"modal fade\" id=\"history".$m."\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\">  
            <div class=\"modal-dialog modal-lg\">
              <div class=\"modal-content\">
                <div class=\"modal-header\">
                  <h5 class=\"modal-title\" id=\"exampleModalLabel\">Data: ".gmdate("d/M/Y H:i:s", $m)."</h5>
                  <button class=\"close\" type=\"button\" data-dismiss=\"modal\" aria-label=\"Close\">
                    <span aria-hidden=\"true\">×</span>
                  </button>
                </div>
                <div class=\"modal-body\">
                  <div class=\"card shadow mb-4\">
                    <div class=\"card-body\">
                      <div class=\"table-responsive\">
                        <table class=\"table table-bordered\" id=\"hisTable\" width=\"100%\" cellspacing=\"0\">
                          <thead>
                              <tr>
                                <th>Identificação do Processo</th>
                                <th>Utilização de CPU em %</th>
                                <th>Memória RAM em %</th>
                                <th>Nome da Aplicação</th>
                              </tr>
                          </thead>";
            $hist = shell_exec('cat history/'.$m);
            $histArray = json_decode($hist, TRUE);
            echo "<tbody>";
            foreach($histArray as $h){
              echo "
                <tr>
                  <th>".$h["pid"]."</th>
                  <th>".$h["cpu"]."</th>
                  <th>".$h["memory"]."</th>
                  <th>".$h["name"]."</th>
                </tr>
              ";
            }
            echo "</tbody>";
            echo "
                              <tfoot>
                                <tr>
                                  <th>Identificação do Processo</th>
                                  <th>Utilização de CPU em %</th>
                                  <th>Memória RAM em %</th>
                                  <th>Nome da Aplicação</th>
                                </tr>
                              </tfoot>
                            </table>
                          </div>
                        </div>
                      </div>          
                    </div>
                  </div>
                </div>
              </div>";
        }
    ?>
    
    <!-- Logout Modal-->
    <div class="modal fade" id="logoutModal" tabindex="-1" role="dialog" aria-labelledby="exampleModalLabel" aria-hidden="true">
      <div class="modal-dialog" role="document">
        <div class="modal-content">
          <div class="modal-header">
            <h5 class="modal-title" id="exampleModalLabel">Deseja mesmo sair?</h5>
            <button class="close" type="button" data-dismiss="modal" aria-label="Close">
              <span aria-hidden="true">×</span>
            </button>
          </div>
          <div class="modal-footer">
            <button class="btn btn-secondary" type="button" data-dismiss="modal">Cancelar</button>
            <a class="btn btn-primary" href="login.php">Sair</a>
          </div>
        </div>
      </div>
    </div>

    <!-- Bootstrap core JavaScript-->
    <script src="vendor/jquery/jquery.min.js"></script>
    <script src="vendor/bootstrap/js/bootstrap.bundle.min.js"></script>

    <!-- Core plugin JavaScript-->
    <script src="vendor/jquery-easing/jquery.easing.min.js"></script>

    <!-- Custom scripts for all pages-->
    <script src="js/sb-admin-2.min.js"></script>

    <!-- Page level plugins -->
    <script src="vendor/chart.js/Chart.min.js"></script>

    <!-- Page level plugins -->
    <script type="text/javascript">
        $( document ).ready(function() {
              $('#empTable').dataTable({
                  "bProcessing": true,
                  "sAjaxSource": "response.php?<?php echo "ip=".$_POST["ip"]."&port=".$_POST["port"]; ?>",
                  "aoColumns": [
                    { mData: 'pid' } ,
                    { mData: 'cpu' },
                    { mData: 'memory' },
                    { mData: 'name' }
                  ]
              });
        });
    </script>
    <script src="vendor/datatables/jquery.dataTables.min.js"></script>
    <script src="vendor/datatables/dataTables.bootstrap4.min.js"></script>

    <!-- Page level custom scripts -->
    <script type="text/javascript" charset="utf8" src="http://ajax.aspnetcdn.com/ajax/jQuery/jquery-1.8.2.min.js"></script>
    <script type="text/javascript" charset="utf8" src="http://ajax.aspnetcdn.com/ajax/jquery.dataTables/1.9.4/jquery.dataTables.min.js"></script>
    <script src="https://canvasjs.com/assets/script/canvasjs.min.js"></script>
    <script>
      window.onload = function () {
        var chart = new CanvasJS.Chart("chartContainer", {
                animationEnabled: true,
                theme: "light2",
                legend:{
                  cursor: "pointer",
                  verticalAlign: "center",
                  horizontalAlign: "right",
                  itemclick: toggleDataSeries
                },
                data: [{
                  type: "column",
                  name: "CPU",
                  indexLabel: "{y}",
                  yValueFormatString: "#0.## %",
                  showInLegend: true,
                  dataPoints: <?php echo json_encode($cpu, JSON_NUMERIC_CHECK); ?>
                },{
                  type: "column",
                  name: "Memory",
                  indexLabel: "{y}",
                  yValueFormatString: "#0.## %",
                  showInLegend: true,
                  dataPoints: <?php echo json_encode($memory, JSON_NUMERIC_CHECK); ?>
                }]
          });
          chart.render();
          function toggleDataSeries(e){
              if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
                  e.dataSeries.visible = false;
              }else{
                  e.dataSeries.visible = true;
              }
                chart.render();
              }
      }
    </script>
    <script src="js/demo/datatables-demo.js"></script>
    <script src="js/demo/chart-bar-demo.js"></script>
  </body>
</html>

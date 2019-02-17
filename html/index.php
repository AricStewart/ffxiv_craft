<?php
require_once __DIR__."/../apiData.inc";
require_once __DIR__."/../ffxivData.inc";

$arguments = [
    'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
    'item'          => FILTER_SANITIZE_SPECIAL_CHARS,
    'crafter'       => FILTER_SANITIZE_SPECIAL_CHARS,
];

$data = filter_input_array(INPUT_GET, $arguments);
if (isset($data['server'])) {
    $server = $data['server'];
}
if (isset($data['item'])) {
    $item = $data['item'];
} else {
    $item = "rakshasa dogi of healing";
}
if (isset($data['crafter'])) {
    $crafter = $data['crafter'];
} else {
    $crafter= "";
}
?>
<html>
<head>
<title>Demonstrating Final Fantasy XIV Crafting Companion</title>

<!-- Bootstrap -->
<link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/css/bootstrap.min.css" integrity="sha384-GJzZqFGwb1QTTN6wy59ffF1BuGJpLSa9DkKMp0DgiMDm4iYMj70gZWKYbI706tWS" crossorigin="anonymous">
<script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo" crossorigin="anonymous"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.6/umd/popper.min.js" integrity="sha384-wHAiFfRlMFy6i5SRaxvfOCifBUQy1xHdJ/yoi7FRNXMRBu5WHdZYu1hA6ZOblgut" crossorigin="anonymous"></script>
<script src="https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/js/bootstrap.min.js" integrity="sha384-B0UglyR+jN6CkvvICOB2joaf5I4l3gm9GU6Hc1og6Ls7i6U/mkkaduKaBhlAXv9k" crossorigin="anonymous"></script>
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<!-- End Bootstrap -->

<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<script src="common.js?version=<?php echo hash_file('sha256', 'common.js');?>"></script>
<script src="crafting.js?version=<?php echo hash_file('sha256', 'crafting.js');?>"></script>
<script src="clipboard.min.js"></script>
</head>
<body>

<!-- Modal -->
<div class="modal" id="refresh_spinner" aria-labelledby="refresh_spinner" aria-hidden="true">
  <div class="modal-dialog modal-dialog-centered">
    <div class="modal-content">
      <div class="modal-body">
        <div class="d-flex justify-content-center">
          <div class="spinner-border" style="width: 3rem; height: 3rem;" role="status">
          </div>
        </div>
      </div>
    </div>
  </div>
</div>

<div>
<div>
<div class="text-center">
<h2>Demonstrating Final Fantasy XIV Crafting Companion</h2>
<a href="masterbook.php">[Master Recipe Books]</a>&nbsp;
<a href="tier.php">[Crafting Tier]</a>
<br><br>
</div>

<div class="m-4">
    <div class="input-group mb-5">
      <div class="input-group-prepend">
        <select class="custom-select" id="server">
    <?php
    $dataset = new FfxivDataSet('..');
    $dataset->loadWorld();
    foreach ($dataset->world as $entry) {
        if (strcasecmp($entry->Name, $server) == 0) {
            echo "<option selected>";
        } else {
            echo "<option>";
        }
        echo $entry->Name."</option>";
    }
    ?>
        </select>
      </div>
      <input id='item' type="text" class="form-control" value='<?php echo $item; ?>'>
      <div class="input-group-append">
        <select class="custom-select" id="crafter">
            <option selected>Any</option>
    <?php
    foreach ($dataset->craftType as $entry) {
        if (strcasecmp($entry, $crafter) == 0) {
            echo "<option selected>";
        } else {
            echo "<option>";
        }
        echo $entry."</option>";
    }
    ?>
        </select>
      </div>
      <div class="input-group-append">
        <input type='button' class="btn btn-outline-secondary" onclick='getData()' value='Get Data'>
      </div>
    </div>
</div>
<div class="progress" id="progress" style="height: 20px; display:none;">
    <div class="progress-bar progress-bar-striped progress-bar-animated active" role="progressbar" aria-valuenow="0" aria-valuemin="0" aria-valuemax="100" style="width: 1%;" id="progress_bar"></div>
</div>

<div id="output">
</div>

<?php
if (isset($data['server']) && isset($data['item'])) {
    echo "<script> getData(); </script>";
}
?>

<footer class="page-footer font-small blue pt-4">
  <div class="footer-copyright text-center py-3">
    &copy; 2019 Copyright: Aric Stewart
    &nbsp;&nbsp;&nbsp;&nbsp;
    <a href="https://github.com/AricStewart/ffxiv_craft"><img src="GitHub-Mark-32px.png"> Check the project out on GitHub</a></span>
  </div>
</footer>

</body>

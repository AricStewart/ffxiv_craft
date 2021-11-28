<?php
require_once __DIR__."/../ffxivData.inc";
require __DIR__.'/../vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();

$arguments = [
'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
'crafter'       => FILTER_SANITIZE_SPECIAL_CHARS,
];

$data = filter_input_array(INPUT_GET, $arguments);
if (isset($data['server'])) {
    $ENV['server'] = $data['server'];
}
if (isset($data['crafter'])) {
    $crafter = $data['crafter'];
} else {
    $crafter = "";
}
?>
<html>
<head>
<title>Demonstrating Final Fantasy XIV Crafting Companion (Tiers)</title>

<!-- Bootstrap -->
<link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/css/bootstrap.min.css" integrity="sha384-GJzZqFGwb1QTTN6wy59ffF1BuGJpLSa9DkKMp0DgiMDm4iYMj70gZWKYbI706tWS" crossorigin="anonymous">
<script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo" crossorigin="anonymous"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.6/umd/popper.min.js" integrity="sha384-wHAiFfRlMFy6i5SRaxvfOCifBUQy1xHdJ/yoi7FRNXMRBu5WHdZYu1hA6ZOblgut" crossorigin="anonymous"></script>
<script src="https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/js/bootstrap.min.js" integrity="sha384-B0UglyR+jN6CkvvICOB2joaf5I4l3gm9GU6Hc1og6Ls7i6U/mkkaduKaBhlAXv9k" crossorigin="anonymous"></script>
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<!-- End Bootstrap -->

<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<script src="common.js?version=<?php echo hash_file('sha256', 'common.js');?>"></script>
<script src="tier.js?version=<?php echo hash_file('sha256', 'tier.js');?>"></script>
<script src="clipboard.min.js"></script>
</head>
<body>

<!-- Modal -->
<div class="modal" id="refresh_spinner" aria-labelledby="refresh_spinner" aria-hidden="true">
  <div class="modal-dialog modal-dialog-centered" style="margin-top: -85px;">
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
<div class="text-center">
<h2>Demonstrating Final Fantasy XIV Crafting Companion (Tiers)</h2>
<a href="index.php">[Recipe Processing]</a>&nbsp;
<a href="masterbook.php">[Master Recipe Books]</a>&nbsp;
<a href="company.php">[Company Workshop]</a>
<br><br>
</div>

<div class="m-4">
    <div class="input-group mb-5">

<div class="input-group-prepend">
<select class="custom-select" id='server'>
<?php
$dataset = new FfxivDataSet('..');
$dataset->loadWorld();
foreach ($dataset->world as $entry) {
    if (strcasecmp($entry->Name, $_ENV['server']) == 0) {
        echo "<option selected>";
    } else {
        echo "<option>";
    }
    echo $entry->Name."</option>";
}
?>
</select>
</div>

<select class="custom-select" id='tier'>
    <option value=1>level 1-5</option>
    <option value=2>level 6-10</option>
    <option value=3>level 11-15</option>
    <option value=4>level 16-20</option>
    <option value=5>level 21-25</option>
    <option value=6>level 26-30</option>
    <option value=7>level 31-35</option>
    <option value=8>level 36-40</option>
    <option value=9>level 41-45</option>
    <option value=10>level 46-50</option>
    <option value=11>level 51-55</option>
    <option value=12>level 56-60</option>
    <option value=13>level 61-65</option>
    <option value=14>level 66-70</option>
    <option value=15>level 71-75</option>
    <option value=16>level 76-80</option>
</select>
      <div class="input-group-append">
        <select class="custom-select" id="crafter">
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

<footer class="page-footer font-small blue pt-4">
  <div class="footer-copyright text-center py-3">
    &copy; 2021 Copyright: Aric Stewart
    &nbsp;&nbsp;&nbsp;&nbsp;
    <a href="https://github.com/AricStewart/ffxiv_craft"><img src="GitHub-Mark-32px.png"> Check the project out on GitHub</a></span>
  </div>
</footer>

<?php include __DIR__.'/tag.php'; ?>

</body>

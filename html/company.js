/* jshint browser: true */

function getDataBlock() {
  var server = encodeURI(document.getElementById('server').value);
  var target = document.getElementById('target').value;
  getDataBlockCore('html_company.php',
    'target=' + target + '&server=' + server);
}

function getDataEvent() {
  var server = encodeURI(document.getElementById('server').value);
  var target = document.getElementById('target').value;
  getDataStreamCore("html_company.php?event=1&target=" + target +
                    "&server=" + server);
}

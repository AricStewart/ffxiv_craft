/* jshint browser: true */

function getDataBlock()
{
  var server = encodeURI(document.getElementById('server').value);
  var tier = document.getElementById('tier').value;
  var crafter = encodeURI(document.getElementById('crafter').value);
  getDataBlockCore(
    'html_tier.php',
    'tier=' + tier + '&server=' + server + '&crafter=' + crafter
  );

}

function getDataEvent()
{
  var server = encodeURI(document.getElementById('server').value);
  var tier = document.getElementById('tier').value;
  var crafter = encodeURI(document.getElementById('crafter').value);
  getDataStreamCore('html_tier.php?event=1&tier=' + tier + '&server=' + server + '&crafter=' + crafter);

}

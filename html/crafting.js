/* jshint browser: true */

function getDataBlock() {
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  var match = encodeURI(document.getElementById('match').value);
  getDataBlockCore('html_craft.php',
    'item=' + item +'&server=' + server + '&crafter=' + crafter + '&match=' +
    match);
}

function getDataEvent() {
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  var match = encodeURI(document.getElementById('match').value);
  resetCrafter(true);
  getDataStreamCore("html_craft.php?event=1&item=" + item +
                    "&server=" + server + "&crafter=" + crafter + '&match=' +
                    match);
}

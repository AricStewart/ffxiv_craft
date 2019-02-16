/* jshint browser: true */

function getDataBlock() {
  document.getElementById('output').innerHTML = "";
  var xhttp = new XMLHttpRequest();
  var server = encodeURI(document.getElementById('server').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  var tier = document.getElementById('tier').value;
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        $("#refresh_spinner").modal("hide");
        var data = JSON.parse(this.responseText);
        fillShortFrame(data);
        new ClipboardJS('.copy_button');
    }
  };
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  xhttp.open('POST', 'html_tier.php', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('tier=' + tier+'&server=' + server + '&crafter=' + crafter);
}

function getDataEvent() {
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  document.getElementById('progress_bar').style.width = "1%";
  document.getElementById('progress_bar').setAttribute('aria-valuenow', 0);
  document.getElementById('progress').style.display = '';
  var server = encodeURI(document.getElementById('server').value);
  var tier = document.getElementById('tier').value;
  var crafter = encodeURI(document.getElementById('crafter').value);
  var source = new EventSource("html_tier.php?event=1&tier=" + tier +
        "&server=" + server + "&crafter=" + crafter);
  source.onmessage = function(event) {
      var data = JSON.parse(event.data);
      if (data.type == "start") {
        document.getElementById('progress_bar').setAttribute('aria-valuemax', data.data);
        document.getElementById('progress_bar').setAttribute('aria-valuenow', 0);
      } else if (data.type == "progress") {
        var w = document.getElementById('progress_bar').getAttribute('aria-valuenow');
        w = parseInt(w)+1;
        document.getElementById('progress_bar').setAttribute('aria-valuenow', w);
        var p = Math.round((w / document.getElementById('progress_bar').getAttribute('aria-valuemax')) * 100);
        p = p + '%';
        document.getElementById('progress_bar').style.width = p;
      } else if (data.type == "info") {
        document.getElementById('progress_bar').innerHTML = data.data;
        console.log(data.data);
      } else if (data.type == "partial") {
        document.getElementById('output').innerHTML = "";
        info = JSON.parse(data.data);
        fillShortFrame(info);
      } else if (data.type == "done") {
        source.close();
        $("#refresh_spinner").modal("hide");
        document.getElementById('progress').style.display = "none";
        info = JSON.parse(data.data);
        document.getElementById('output').innerHTML = "";
        fillShortFrame(info);
        new ClipboardJS('.copy_button');
      }
  };
}

function getData() {
    document.getElementById('output').innerHTML = "";
    if(typeof(EventSource) !== "undefined") {
      getDataEvent();
    } else {
      getDataBlock();
    }
}

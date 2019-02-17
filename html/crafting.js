/* jshint browser: true */

function fillCraftFrame(data)
{
    if (data.Info !== null) {
        document.getElementById('crafter').value = data.Info.CraftTypeName;
        var a = document.getElementById('crafter');
        for (i=0, n=a.options.length;i<n;i++) {
            l = document.getElementById('crafter').options[i].value;
            if (l == 'Any' || data.Info.AllCraftTypes.includes(l)) {
                document.getElementById('crafter').options[i].disabled = false;
            } else {
                document.getElementById('crafter').options[i].disabled = true;
            }
        }
    } else {
        document.getElementById('crafter').value = "Any";
        for (i=0, n=document.getElementById('crafter').options.length;i<n;i++) {
            l = document.getElementById('crafter').options[i].value;
            if (l == 'Any') {
                document.getElementById('crafter').options[i].disabled = false;
            } else {
                document.getElementById('crafter').options[i].disabled = true;
            }
        }
    }

    content = '<div class="card m-5" id="RecipeParent'+data.ID+'">  <div class="card-body">';
    content += fillRecipeFrame(data, false);
    content += fillRecipe(data, true);
    content += "</div>\n";

    document.getElementById('output').innerHTML  = content;
    new ClipboardJS('.copy_button');

    $('#collapse'+data.ID).on('hide.bs.collapse', function () {
      $('#RecipeArrow'+data.ID).toggleClass('fa-caret-up fa-caret-down');
    });

    $('#collapse'+data.ID).on('show.bs.collapse', function () {
      $('#RecipeArrow'+data.ID).toggleClass('fa-caret-down fa-caret-up');
    });
}

function getDataBlock() {
  document.getElementById('output').innerHTML = "";
  var xhttp = new XMLHttpRequest();
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        $("#refresh_spinner").modal("hide");
        var data = JSON.parse(this.responseText);
        fillCraftFrame(data);
    }
  };
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  xhttp.open('POST', 'html_craft.php', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('item=' + item +'&server=' + server + '&crafter=' + crafter);
}

function getDataEvent() {
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  document.getElementById('progress_bar').style.width = "1%";
  document.getElementById('progress_bar').setAttribute('aria-valuenow', 0);
  document.getElementById('progress').style.display = '';
  var server = encodeURI(document.getElementById('server').value);
  var item = encodeURI(document.getElementById('item').value);
  var crafter = encodeURI(document.getElementById('crafter').value);
  var source = new EventSource("html_craft.php?event=1&item=" + item +
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
        console.log(data.data);
      } else if (data.type == "partial") {
        document.getElementById('output').innerHTML = "";
        item = JSON.parse(data.data);
        fillCraftFrame(item);
      } else if (data.type == "done") {
        source.close();
        $("#refresh_spinner").modal("hide");
        document.getElementById('progress').style.display = "none";
        if (data.data === "[]") {
            var name = document.getElementById('item').value;
            alert("Failed to find recipe for item  '" + name + "'");
        }
        item = JSON.parse(data.data);
        fillCraftFrame(item);
      }
  };
}

function getData() {
    if(typeof(EventSource) !== "undefined") {
      getDataEvent();
    } else {
      getDataBlock();
    }
}

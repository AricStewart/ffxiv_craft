/* jshint browser: true */

function getBadge(order, value) {
    var i = order.indexOf(value);
    if (order.length == 3 && i == 2) {
        return '<span class="badge badge-danger">';
    } else if ( order.length >= 2 && i == 1) {
        return '<span class="badge badge-primary">';
    } else {
        return '<span class="badge badge-success">';
    }
}

function printLine(line, tab)
{
    var server = document.getElementById('server').value;
    var output = '<ul class="list-style">\n';
    line.forEach(function (l) {
        output += '<li>\n';
        if (l.craftCost > 0) {
            output += '<a href="index.php?server='+server+'&item='+l.id+
            '&crafter='+l.craftedBy+'" target="_blank">';
        } else {
            output += '<a href="https://mogboard.com/#'+server+','+l.id+'" target="_blank">';
        }
       output += l.name+'</a> (x'+l.count+') -> ';

        var low = [];
        if (l.marketCost > 0) low.push(l.marketCost);
        if (l.craftCost > 0) low.push(l.craftCost);
        if (l.shopCost > 0) low.push(l.shopCost);
        low.sort(function(a, b){return a - b;});

        if (l.marketCost > 0) {
            output += getBadge(low, l.marketCost);
            output += 'market</span>&nbsp;';
            output += l.marketCost.toLocaleString() + " gil ";
            if (l.marketHQ) {
                output += "<img src='hq.png'>";
            }
        } else {
            output += "UNAVAILABLE";
        }
        if (l.craftCost > 0) {
            output += '&nbsp;' + getBadge(low, l.craftCost);
            output += 'crafted</span>&nbsp(' +
              '<a href="index.php?server='+server+'&item='+l.id +
              '&crafter='+l.craftedBy+'" target="_blank">'+ l.craftedBy +
              '</a>) ' + l.craftCost.toLocaleString() + " gil";
        }
        if (l.shopCost > 0) {
            output += '&nbsp;' + getBadge(low, l.shopCost);
            output += 'vendor</span>&nbsp;';
            output +='<a href="http://www.garlandtools.org/db/#item/' +
                    l.id + '" target="_blank">' +
                    l.shopCost.toLocaleString() + "</a> gil";
        }
        if (l.bits.length > 0) {
            output += printLine(l.bits, tab+1);
        }
        output += "</li>\n";
    });
    output += '</ul>\n';
    return output;
}

function fillRecipe(data, open)
{
    var collapse = 'collapse' + data.ID;
    content = "<div class='card text-left'>\n";
    content +=  '<div class="card-header" id="heading'+data.ID+'">' +
               '   <h5 class="mb-0"> '+
               '     <button class="btn btn-link" type="button" data-toggle="collapse" data-target="#' + collapse + '" aria-expanded="true" aria-controls="' + collapse+ '">'+
               '       Recipe'+
               '     </button>';
    if (open) {
        content += '    <i id="RecipeArrow' + data.ID + '" class="fa fa-caret-up"></i>';
    } else {
        content += '    <i id="RecipeArrow' + data.ID + '" class="fa fa-caret-down"></i>';
    }
    content += '   </h5>' +
               ' </div>';
    if (open) {
        content += '<div id="' + collapse + '" class="collapse show"';
    } else {
        content += '<div id="' + collapse+ '" class="collapse"';
    }
    content += 'aria-labelledby="heading' + data.ID + '" data-parent="#RecipeParent'+data.ID+'">' +
               '   <div class="card-body">';
    content += printLine(data.Recipe, 0);
    content += ' </div> </div> </div>\n';

    return content;
}

function fillRecipeFrame(data, linkback, profit, marketboard)
{
    if (data === null) {
        return;
    }
    if (data.Cost === undefined) {
        marketCost = 'CALCULATING...';
    } else if (data.Cost.Market <= 0) {
        marketCost =  'UNAVAILABLE';
    } else {
        marketCost = data.Cost.Market.toLocaleString() + " gil";
    }

    if (data.Cost === undefined){
        optimalCost = 'CALCULATING...';
    } else if (data.Cost.Optimal <= 0) {
        optimalCost =  'UNAVAILABLE';
    } else {
        optimalCost = data.Cost.Optimal.toLocaleString() + " gil";
    }

    if (data.Cost === undefined) {
        shopCost = 'CALCULATING...';
    } else if (data.Cost.Shop <= 0) {
        shopCost =  'UNAVAILABLE';
    } else {
        shopCost = '<a href="http://www.garlandtools.org/db/#item/'+
            data.ID + '" target="_blank">' + data.Cost.Shop.toLocaleString() +
            "</a> gil";
    }

    if (marketboard) {
        if (data.Cost === undefined) {
            boardCost = 'CALCULATING...';
        } else if (data.Cost.Marketboard === undefined ||
                   data.Cost.Marketboard <= 0) {
            boardCost = 'UNAVAILABLE';
        } else {
            boardCost = data.Cost.Marketboard.toLocaleString() + " gil";
        }
    }

    if (data.Cheap === undefined) {
        cheap = 'CALCULATING...';
    } else if (data.Cheap.LQ === null) {
        cheap =  'UNAVAILABLE';
    } else {
        cheap = data.Cheap.LQ.Item.PricePerUnit.toLocaleString()+" gil";
        cheap +=" ("+data.Cheap.LQ.Count+" listings)";
    }
    if (data.Cheap !== undefined && data.Cheap.HQ !== null) {
        cheap += " / <img src='hq.png'>";
        cheap += data.Cheap.HQ.Item.PricePerUnit.toLocaleString()+" gil";
        cheap +=" ("+data.Cheap.HQ.Count+" listings)";
    }

    if (data.Week === undefined) {
        recent = 'CALCULATING...';
    } else if (data.Recent.LQ === null) {
        recent =  'UNAVAILABLE';
    } else {
        recent = data.Recent.LQ.PricePerUnit.toLocaleString()+" gil";
    }
    if (data.Recent !== undefined && data.Recent.HQ !== null) {
        recent += " / <img src='hq.png'>";
        recent += data.Recent.HQ.PricePerUnit.toLocaleString()+" gil";
    }

    if (data.Week === undefined) {
        week = 'CALCULATING...';
    } else if (data.Week.LQ.Average === 0) {
        week =  'UNAVAILABLE';
    } else {
        week = data.Week.LQ.Average.toLocaleString()+" gil";
        sales = data.Week.LQ.Count;
        plu = 's';
        if (sales === 1) {
            plu = '';
        }
        week +=" ( "+sales.toLocaleString()+" sale"+plu+" )";
    }
    if (data.Week !== undefined && data.Week.HQ.Average !== 0) {
        week += " / <img src='hq.png'>";
        week += data.Week.HQ.Average.toLocaleString()+" gil";
        sales = data.Week.HQ.Count;
        plu = 's';
        if (sales === 1) {
            plu = '';
        }
        week +=" ( "+sales.toLocaleString()+" sale"+plu+" )";
    }

    var dataName = "";
    var server = encodeURI(document.getElementById('server').value);
    if (linkback) {
        if (data.Info !== null) {
            var item = encodeURI(data.ID);
            dataName = '<a href="index.php?server='+server+'&item='+item +
                        '&crafter='+data.Info.CraftTypeName+'" target="_blank">' +
                        data.Name + '</a>';
            if (data.Info.Result.Amount > 1 || data.Count > 1) {
                dataName += ' x' + data.Info.Result.Amount * data.Count;
            }
        } else {
            dataName = data.Name;
        }
    } else{
        dataName ='<a href="http://www.garlandtools.org/db/#item/' +
            data.ID + '" target="_blank">' + data.Name + "</a>";
        if (data.Info !== null && data.Info.Result.Amount > 1) {
            dataName += ' x' + data.Info.Result.Amount;
        }
    }
    dataName += '<sup><input type="image" class="copy_button" ' +
                'src="clipboard.png" data-clipboard-text="' +
                data.Name + '"></sup>';

    var subText = '';
    if (data.Info !== null) {
        subText = data.Info.CraftTypeName + ' - lvl ' +
                  data.Info.RecipeLevel.ClassJobLevel;

        if (data.Info.Book !== null) {
            subText += " from '<a href=\"masterbook.php?server=" + server +
                "&item="+ encodeURIComponent(data.Info.Book.Name) +
                "\" target='_blank'>" + data.Info.Book.Name + "</a>'";
        }
    }

    var sect =
    '    <h2 class="card-title text-center">'+dataName+'</h2>' +
    '    <h4 class="text-center">'+subText+'</h4>' +
    '    <ul class="list-unstyled text-left">' +
    '       <li>Recent: '+recent+'</li>'+
    '       <li>Weekly Average: '+week+'</li>'+
    '       <li>Current: '+cheap+"</li>"+
    '    </ul>' +
    '  <ul class="list-unstyled text-left">' +
    '    <li>Vendor Cost: ' +shopCost+"</li>";
    if (marketboard) {
        sect += '    <li>MarketBoard Cost: '+boardCost+"</li>";
    }
    sect +=
    '    <li>Craft at Market Cost: '+marketCost+'</li>'+
    '    <li>Craft at Optimal Cost: '+optimalCost+"</li>"+
    '  </ul>';

    if (profit && data.Profit !== undefined &&(data.Profit.LQ > 0 || data.Profit.HQ > 0) ){
        sect += '<ul class="list-unstyled text-left">';
        if (data.Profit.LQ > 0) {
            sect += '<li><b>Possible Profit</b>: '+data.Profit.LQ.toLocaleString()+" gil ("+Math.round(data.Profit["LQ%"]*100)+"%)</li>";
        }
        if (data.Profit.HQ > 0) {
            sect += "<li><b>Possible Profit</b>: <img src='hq.png'>"+data.Profit.HQ.toLocaleString()+" gil ("+Math.round(data.Profit["HQ%"]*100)+"%)</li>";
        }
        sect += "</ul>\n";
    }
    return sect;
}

function recipeCard(data, linkback, open) {
    var sect = '<div class="card m-5 shadow-lg" id="RecipeParent'+data.ID+'"> <div class="card-body">';
    sect += fillRecipeFrame(data, linkback, true, false);
    sect += fillRecipe(data, open);
    sect += '</div></div>';
    return sect;
}

function setupCollapse(data) {
    $('#collapse'+data.ID).on('hide.bs.collapse', function () {
      $('#RecipeArrow'+data.ID).toggleClass('fa-caret-up fa-caret-down');
    });

    $('#collapse'+data.ID).on('show.bs.collapse', function () {
      $('#RecipeArrow'+data.ID).toggleClass('fa-caret-down fa-caret-up');
    });
}

function resetCrafter(disable)
{
    var element = document.getElementById('crafter');
    if (!element) {
        return;
    }

    element.value = "Any";
    for (i=0, n=element.options.length;i<n;i++) {
        l = element.options[i].value;
        if (l == 'Any') {
            element.options[i].disabled = false;
        } else {
            element.options[i].disabled = disable;
        }
    }
}

function updateCrafter(data)
{
    var element = document.getElementById('crafter');
    if (!element) {
        return;
    }

    if (data && data.Info !== null) {
        element.value = data.Info.CraftTypeName;
        for (i=0, n=element.options.length;i<n;i++) {
            l = element.options[i].value;
            if (l == 'Any' || data.Info.AllCraftTypes.includes(l)) {
                element.options[i].disabled = false;
            }
        }
    }
}

function fillCompanyRecipeFrame(data)
{
    if (data === null) {
        return;
    }
    if (data.Cost === undefined) {
        marketCost = 'CALCULATING...';
    } else if (data.Cost.Market <= 0) {
        marketCost =  'UNAVAILABLE';
    } else {
        marketCost = data.Cost.Market.toLocaleString() + " gil";
    }

    if (data.Cost === undefined){
        optimalCost = 'CALCULATING...';
    } else if (data.Cost.Optimal <= 0) {
        optimalCost =  'UNAVAILABLE';
    } else {
        optimalCost = data.Cost.Optimal.toLocaleString() + " gil";
    }

    if (data.Cost === undefined) {
        boardCost = 'CALCULATING...';
    } else if (data.Cost.Marketboard <= 0) {
        boardCost = 'UNAVAILABLE';
    } else {
        boardCost = data.Cost.Marketboard.toLocaleString() + " gil";
    }

    if (data.marketCost === undefined) {
        cheap = 'CALCULATING...';
    } else if (data.marketCost  === null) {
        cheap =  'UNAVAILABLE';
    } else {
        cheap = data.marketCost.toLocaleString()+" gil";
        cheap +=" ("+data.MarketNumber+" listings)";
    }

    if (data.Recent === undefined) {
        recent = 'CALCULATING...';
    } else if (data.Recent !== null) {
        recent = data.Recent.PricePerUnit.toLocaleString()+" gil ( Purchased ";
        var ts = new Date(data.Recent.PurchaseDate*1000);
        recent += ts.toString() + ")";
    }

    if (data.Week === undefined) {
        week = 'CALCULATING...';
    } else if (data.Week.PricePerUnit === 0) {
        week =  'UNAVAILABLE';
    } else {
        week = data.Week.Minimum.toLocaleString()+" gil";
        sales = data.Week.Count;
        plu = 's';
        if (sales === 1) {
            plu = '';
        }
        week +=" ( "+sales.toLocaleString()+" sale"+plu+" )";
    }

    var server = encodeURI(document.getElementById('server').value);
    var dataName ='<a href="http://www.garlandtools.org/db/#item/' +
        data.ID + '" target="_blank">' + data.Name + "</a>";
    dataName += '<sup><input type="image" class="copy_button" ' +
                'src="clipboard.png" data-clipboard-text="' +
                data.Name + '"></sup>';

    var sect =
    '<div class="card m-5 shadow-lg">' +
    '  <h2 class="card-title text-center">'+dataName+'</h2>' +
    '<div class="card-body">' +
    '  <ul class="list-unstyled text-left">' +
    '     <li>Recent: '+recent+'</li>'+
    '      <li>Weekly Average: '+week+'</li>'+
    '     <li>Current: '+cheap+"</li>"+
    '  </ul>' +
    '  <ul class="list-unstyled text-left">' +
    '    <li>MarketBoard Cost: '+boardCost+"</li>" +
    '    <li>Craft at Market Cost: '+marketCost+'</li>'+
    '    <li>Craft at Optimal Cost: '+optimalCost+"</li>"+
    '  </ul></div></div>';

    return sect;
}

function fillCompanyStageFrame(data)
{
    if (data === null) {
        return;
    }
    if (data.Cost === undefined) {
        marketCost = 'CALCULATING...';
    } else if (data.Cost.Market <= 0) {
        marketCost =  'UNAVAILABLE';
    } else {
        marketCost = data.Cost.Market.toLocaleString() + " gil";
    }

    if (data.Cost === undefined){
        optimalCost = 'CALCULATING...';
    } else if (data.Cost.Optimal <= 0) {
        optimalCost =  'UNAVAILABLE';
    } else {
        optimalCost = data.Cost.Optimal.toLocaleString() + " gil";
    }

    if (data.Cost === undefined) {
        boardCost = 'CALCULATING...';
    } else if (data.Cost.Marketboard <= 0) {
        boardCost = 'UNAVAILABLE';
    } else {
        boardCost = data.Cost.Marketboard.toLocaleString() + " gil";
    }

    var sect =
    '  <ul class="list-unstyled text-left">' +
    '    <li>MarketBoard Cost: '+boardCost+"</li>" +
    '    <li>Craft at Market Cost: '+marketCost+'</li>'+
    '    <li>Craft at Optimal Cost: '+optimalCost+"</li>"+
    '  </ul>';

    return sect;
}

function fillOutputFrame(dataset)
{
    var sect;
    if ("Parts" in dataset) {
        sect = fillCompanyRecipeFrame(dataset);
        document.getElementById('output').innerHTML += sect;
        dataset.Parts.forEach(function(part) {
            var i = 1;
            part.Process.forEach(function(process) {
                document.getElementById('output').innerHTML += '<hr>';
                var sect = '<div class="card m-5 shadow-lg" id="">' +
               '<div class="card-header">' +
               '   <h5 class="mb-0"> Stage ' + i +
               '   </h5>' +
               ' </div>';
                i += 1;
                sect += '<div class="card-body">';
                sect += fillCompanyStageFrame(process);
                process.Set.forEach(function(item) {
                    sect += '<div class="card m-5  id="RecipeParent'+item.Recipe.ID+'"> <div class="card-body">';

                    sect += '<div class="card m-5 shadow-lg" id="RecipeParent'+item.Recipe.ID+'"> <div class="card-body">';
                    sect += fillRecipeFrame(item.Recipe, true, false, true);
                    sect += fillRecipe(item.Recipe, false);
                    sect += '</div></div>';

                    sect += '</div>';
                    sect += '</div>';
                });
                document.getElementById('output').innerHTML += sect;
            });
        });
        return;
    }
    if (Array.isArray(dataset)) {
        if (dataset.length == 1) {
            return fillOutputFrame(dataset[0]);
        }
        dataset.forEach(function(data) {
            updateCrafter(data);
            sect = '<div class="card m-5 shadow-lg" id="RecipeParent'+data.ID+'"> <div class="card-body">';
            sect += recipeCard(data, true, false);
            document.getElementById('output').innerHTML += sect;
        });
        dataset.forEach(function(data) {
            setupCollapse(data);
        });
    } else {
        updateCrafter(dataset);
        sect = '<div class="card m-5" id="RecipeParent'+dataset.ID+'"> <div class="card-body">';
        sect += recipeCard(dataset, false, true);
        document.getElementById('output').innerHTML += sect;

        setupCollapse(dataset);
    }
    new ClipboardJS('.copy_button');
}

function copyItem(element) {
  var copyText = document.getElementById(element);
  copyText.select();
  document.execCommand("copy");
}

function getDataBlockCore(uri, params) {
  document.getElementById('output').innerHTML = "";
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        $("#refresh_spinner").modal("hide");
        var data = JSON.parse(this.responseText);
        fillOutputFrame(data);
    }
  };
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});
  xhttp.open('POST', uri, true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send(params);
}

function getDataStreamCore(uri) {
  $("#refresh_spinner").modal({backdrop: 'static', keyboard: false});

  var bar = document.getElementById('progress_bar');
  bar.style.width = "1%";
  bar.setAttribute('aria-valuenow', 0);
  document.getElementById('progress').style.display = '';
  var source = new EventSource(uri);
  source.onmessage = function(event) {
      var data = JSON.parse(event.data);
      if (data.type == "start") {
        bar.setAttribute('aria-valuemax', data.data);
        bar.setAttribute('aria-valuenow', 0);
      } else if (data.type == "progress") {
        var w = bar.getAttribute('aria-valuenow');
        w = parseInt(w)+1;
        bar.setAttribute('aria-valuenow', w);
        var p = Math.round((w / bar.getAttribute('aria-valuemax')) * 100);
        p = p + '%';
        bar.style.width = p;
      } else if (data.type == "info") {
        bar.innerHTML = data.data;
        console.log(data.data);
      } else if (data.type == "partial") {
        document.getElementById('output').innerHTML = "";
        info = JSON.parse(data.data);
        fillOutputFrame(info);
      } else if (data.type == "done") {
        source.close();
        $("#refresh_spinner").modal("hide");
        document.getElementById('progress').style.display = "none";
        if (data.data === "[]") {
            alert("Failed to find recipe for item: "+uri);
        }
        info = JSON.parse(data.data);
        document.getElementById('output').innerHTML = "";
        fillOutputFrame(info);
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

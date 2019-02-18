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
    content += ' </div> </div> </div>';
    content += "</div>";
    content += "</div>";
    content += "</div>\n";

    return content;
}

function fillRecipeFrame(data, linkback)
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
        shopCost = data.Cost.Shop.toLocaleString() + " gil";
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
        var item = encodeURI(data.ID);
        dataName = '<a href="index.php?server='+server+'&item='+item +
                    '&crafter='+data.Info.CraftTypeName+'" target="_blank">' +
                    data.Name + '</a>';
        if (data.Info.Result.Amount > 1) {
            dataName += ' x' + data.Info.Result.Amount;
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
    '    <li>Vendor Cost: '+shopCost+"</li>" +
    '    <li>Craft at Market Cost: '+marketCost+'</li>'+
    '    <li>Craft at Optimal Cost: '+optimalCost+"</li>"+
    '  </ul>';

    if (data.Profit !== undefined &&(data.Profit.LQ > 0 || data.Profit.HQ > 0) ){
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

function fillShortFrame(dataset)
{
    dataset.forEach(function(data) {
        sect = '<div class="card m-5 shadow-lg" id="RecipeParent'+data.ID+'"> <div class="card-body">';
        sect += fillRecipeFrame(data, true);
        sect += fillRecipe(data, false);
        sect += '</div></div>';
        document.getElementById('output').innerHTML += sect;
    });
    dataset.forEach(function(data) {
        $('#collapse'+data.ID).on('hide.bs.collapse', function () {
          $('#RecipeArrow'+data.ID).toggleClass('fa-caret-up fa-caret-down');
        });

        $('#collapse'+data.ID).on('show.bs.collapse', function () {
          $('#RecipeArrow'+data.ID).toggleClass('fa-caret-down fa-caret-up');
        });
    });
    new ClipboardJS('.copy_button');
}

function copyItem(element) {
  var copyText = document.getElementById(element);
  copyText.select();
  document.execCommand("copy");
}

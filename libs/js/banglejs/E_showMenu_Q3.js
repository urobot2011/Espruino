(function(items) {
  g.reset().clearRect(Bangle.appRect); // clear if no menu supplied
  Bangle.setLCDPower(1); // ensure screen is on
  if (!items) {
    Bangle.setUI();
    return;
  }
  var menuItems = Object.keys(items);
  var options = items[""];
  if (options) menuItems.splice(menuItems.indexOf(""),1);
  if (!(options instanceof Object)) options = {};
  options.fontHeight = options.fontHeight||21;
  if (options.selected === undefined)
    options.selected = 0;
  var ar = Bangle.appRect;
  var x = ar.x;
  var x2 = ar.x2;
  var y = ar.y;
  var y2 = ar.y2 - 12; // padding at end for arrow
  if (options.title)
    y += 22;
  var loc = require("locale");
  var l = {
    draw : function() {
      g.reset();
      g.setColor(cFg);
      g.setFont('Vector',18).setFontAlign(0,-1,0);
      if (options.title) {
        g.drawString(options.title,(x+x2)/2,y-options.fontHeight-2);
        g.drawLine(x,y-2,x2,y-2);
      }
      var rows = 0|Math.min((y2-y) / options.fontHeight,menuItems.length);
      var idx = E.clip(options.selected-(rows>>1),0,menuItems.length-rows);
      if (idx!=l.lastIdx) rowmin=undefined; // redraw all if we scrolled
      l.lastIdx = idx;      
      var more = (idx+rows)<menuItems.length;
      var iy = y;
      g.reset().setFontAlign(0,-1,0).setFont('12x20');
      if (options.predraw) options.predraw(g);
      if (rowmin===undefined && options.title)
        g.drawString(options.title,(x+x2)/2,y-21).drawLine(x,y-2,x2,y-2).
          setColor(g.theme.fg).setBgColor(g.theme.bg);
      iy += 12;
      g.setColor((idx>0)?g.theme.fg:g.theme.bg).fillPoly([72,iy,104,iy,88,iy-12]);      
      if (rowmin!==undefined) {
        if (idx<rowmin) {
          iy += options.fontHeight*(rowmin-idx);
          idx=rowmin;
        }
        if (idx+rows>rowmax) {
          rows = 1+rowmax-rowmin;
        }
      }
      while (rows--) {
        var name = menuItems[idx];
        var item = items[name];
        var hl = (idx==options.selected && !l.selectEdit);
        g.setColor(hl ? g.theme.bgH : g.theme.bg);
        g.fillRect(x,iy,x2,iy+options.fontHeight-1);
        g.setColor(hl ? g.theme.fgH : g.theme.fg);
        g.setFontAlign(-1,-1);
        g.drawString(loc.translate(name),x+1,iy+1);
        if ("object" == typeof item) {
          var xo = x2;
          var v = item.value;
          if (item.format) v=item.format(v);
          if (l.selectEdit && idx==options.selected) {
            xo -= 24 + 1;
            g.setColor(g.theme.bgH).fillRect(xo-(g.stringWidth(v)+4),iy,x2,iy+options.fontHeight-1);
            g.setColor(g.theme.fgH).drawImage("\x0c\x05\x81\x00 \x07\x00\xF9\xF0\x0E\x00@",xo,iy+(options.fontHeight-10)/2,{scale:2});
          }
          g.setFontAlign(1,-1);
          g.drawString(v,xo-2,iy+1);
        }
        g.setColor(g.theme.fg);
        iy += options.fontHeight;
        idx++;
      }
      g.setFontAlign(-1,-1);      
      g.setColor(more?g.theme.fg:g.theme.bg).fillPoly([72,166,104,166,88,174]);
      g.flip();
    },
    select : function() {
      var item = items[menuItems[options.selected]];
      if ("function" == typeof item) item(l);
      else if ("object" == typeof item) {
        // if a number, go into 'edit mode'
        if ("number" == typeof item.value)
          l.selectEdit = l.selectEdit?undefined:item;
        else { // else just toggle bools
          if ("boolean" == typeof item.value) item.value=!item.value;
          if (item.onchange) item.onchange(item.value);
        }
        l.draw();
      }
    },
    move : function(dir) {
      var item = l.selectEdit
      if (item) {
        item.value -= (dir||1)*(item.step||1);
        if (item.min!==undefined && item.value<item.min) item.value = item.wrap ? item.max : item.min;
        if (item.max!==undefined && item.value>item.max) item.value = item.wrap ? item.min : item.max;
        if (item.onchange) item.onchange(item.value);
        l.draw(options.selected,options.selected);
      } else {
        var lastSelected=options.selected;
        options.selected = (dir+options.selected+menuItems.length)%menuItems.length;
        l.draw(Math.min(lastSelected,options.selected), Math.max(lastSelected,options.selected));
      }
    }
  };
  var selbut = -1;
  var butdefs = [{x1:8,y1:150,x2:44,y2:175,poly:[8,175,26,150,44,175]},
                 {x1:69,y1:150,x2:105,y2:175,poly:[69,150,105,150,69,175,105,175]},
                 {x1:130,y1:150,x2:166,y2:175,poly:[130,150,166,150,148,175]}];
  var drawButton = function(d,sel){
       (sel?g.setColor(3):g.setColor(1)).fillRect(d.x1,d.y1,d.x2,d.y2);
       g.setColor(-1).fillPoly(d.poly).flip();
  };
  for(var i=0;i<3;i++)drawButton(butdefs[i],false);
  var isPressed = function(p,n) {
      var d = butdefs[n];
      var bb = (p.x>d.x1 && p.x<d.x2 && p.y>130);
      if (bb) {selbut=n; drawButton(d,true);setTimeout(()=>{drawButton(d,false);},150);}
      return bb;
  };
  Bangle.buttons = function(p){
    if (isPressed(p,0)) l.move(-1);
    else if (isPressed(p,1)) l.select(); 
    else if (isPressed(p,2)) l.move(1);
    else selbut=-1;
  };
  l.draw();
  E.on("touch",Bangle.buttons);
  return l;  
})

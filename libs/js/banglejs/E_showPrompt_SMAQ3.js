/* options = {
  title: text
  buttons : {"Yes":true,"No":false}
} */
(function(msg,options) {
  var FSIZE = 18;
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var btns = Object.keys(options.buttons);
  if (!options.selected)
    options.selected = 0;
  function draw() {
    g.reset().setFont("Vector",FSIZE).setFontAlign(0,0);
    var W = g.getWidth();
    var H = g.getHeight();
    var title = options.title;
    if (title) {
      g.drawString(title,W/2,24);
      var w = (g.stringWidth(title)+16)/2;
      g.fillRect((W/2)-w,34,(W/2)+w,34);
    }
    var lines = msg.split("\n");
    var offset = (H - lines.length*FSIZE)/2;
    lines.forEach((line,y)=>
      g.drawString(line,W/2,offset + y*FSIZE));    
    var buttonWidths = 0;
    var buttonPadding = 48;
    btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(btn));
    var x = (W-buttonWidths)/2;
    var y = H-20;
    btns.forEach((btn,idx)=>{
      var w = g.stringWidth(btn);
      x += (buttonPadding+w)/2;      
      var bw = 2+w/2;
      var poly = [x-bw,y-12,
                  x+bw,y-12,
                  x+bw+4,y-8,
                  x+bw+4,y+8,
                  x+bw,y+12,
                  x-bw,y+12,
                  x-bw-4,y+8,
                  x-bw-4,y-8,
                  x-bw,y-12];
      g.setColor(idx==options.selected ? 3 : 0).fillPoly(poly).setColor(-1).drawPoly(poly).drawString(btn,x,y+1);
      x += (buttonPadding+w)/2;
    });
    g.setColor(-1).flip();  // turn screen on
  }
  if (Bangle.prompt) {E.removeListener("touch",Bangle.prompt); Bangle.prompt=undefined;}
  g.clear(1); // clear screen
  if (!msg) {
    return Promise.resolve();
  }
  draw();
  var RES = null;
  Bangle.prompt =  function(p){
    var x = p.x; var y = p.y;
    if (y<120)return -1;
    if (btns.length==1) {
      if (x>60 && x<120) {
        E.showPrompt();
        return RES(options.buttons[btns[options.selected]]);
      } 
      return;
    } else {
      if (x<70) {
        if (options.selected != 0) {
          options.selected = 0;
          draw();
        } else {
          E.showPrompt();
          return RES(options.buttons[btns[options.selected]]);
        }
      } else if (x>110) {
        if (options.selected!=1) {
          options.selected=1;
          draw(); 
        } else {
          E.showPrompt();
          return RES(options.buttons[btns[options.selected]]);
        }
      }
    }
    return;
  };
  return new Promise(resolve=>{
    RES = resolve;
    E.on("touch",Bangle.prompt);
  });
})

E.showMessage = function(msg,title) {
    g.clear(1); // clear screen
    g.setFont("Vector",18).setFontAlign(0,0);
    var W = g.getWidth();
    var H = g.getHeight()-26;
    if (title) {
      g.drawString(title,W/2,18);
      var w = (g.stringWidth(title)+12)/2;
      g.fillRect((W/2)-w,26,(W/2)+w,26);
    }
    var lines = msg.split("\n");
    var offset = 26+(H - lines.length*18)/2 ;
    lines.forEach((line,y)=>g.drawString(line,W/2,offset+y*18));
    g.flip();
};
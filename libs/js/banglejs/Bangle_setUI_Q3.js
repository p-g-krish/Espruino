(function(mode, cb) {
  var options = {};
  if ("object"==typeof mode) {
    options = mode;
    mode = options.mode;
  }
  if (global.WIDGETS && WIDGETS.back)
    WIDGETS.back.remove();
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    delete Bangle.btnWatches;
  }
  if (Bangle.dragHandler) {
    Bangle.removeListener("drag", Bangle.dragHandler);
    delete Bangle.dragHandler;
  }
  if (Bangle.touchHandler) {
    Bangle.removeListener("touch", Bangle.touchHandler);
    delete Bangle.touchHandler;
  }
  if (Bangle.uiRemove) {
    Bangle.uiRemove();
    delete Bangle.uiRemove;
  }  
  function b() {
    try{Bangle.buzz(30);}catch(e){}
  }
  if (!mode) return;
  else if (mode=="updown") {
    var dy = 0;    
    Bangle.dragHandler = e=>{
      dy += e.dy;
      if (!e.b) dy=0;
      while (Math.abs(dy)>32) {
        if (dy>0) { dy-=32; cb(1) }
        else { dy+=32; cb(-1) }
        Bangle.buzz(20);
      }
    };
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1}),
    ];
  } else if (mode=="leftright") {
    var dx = 0;    
    Bangle.dragHandler = e=>{
      dx += e.dx;
      if (!e.b) dx=0;
      while (Math.abs(dx)>32) {
        if (dx>0) { dx-=32; cb(1) }
        else { dx+=32; cb(-1) }
        Bangle.buzz(20);
      }
    };
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1}),
    ];
  } else if (mode=="clock") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"falling"})
    ];
  } else if (mode=="clockupdown") {
    Bangle.CLOCK=1;
    Bangle.touchHandler = (d,e) => {
      if (e.x < 120) return;
      b();cb((e.y > 88) ? 1 : -1);
    };
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"falling"})
    ];
  } else if (mode=="touch") {
    Bangle.touchHandler = (_,e) => {b();cb(e);};
    Bangle.on("touch", Bangle.touchHandler);
  } else if (mode=="custom") {
    if (options.touch) {
      Bangle.touchHandler = options.touch;
      Bangle.on("touch", Bangle.touchHandler);
    }
    if (options.drag) {
      Bangle.dragHandler = options.drag;
      Bangle.on("drag", Bangle.dragHandler);
    }
  } else
    throw new Error("Unknown UI mode");
  if (options.back) {
    var touchHandler = (_,e) => {
      if (e.y<24 && e.x<48) options.back();
    };
    Bangle.on("touch", touchHandler);    
    var btnWatch = setWatch(function() {
      options.back();
    }, BTN1, {edge:"falling"});
    WIDGETS = Object.assign({back:{ 
      area:"tl", width:24,       draw:e=>g.reset().setColor("#f00").drawImage(atob("GBiBAAAYAAH/gAf/4A//8B//+D///D///H/P/n+H/n8P/n4f/vwAP/wAP34f/n8P/n+H/n/P/j///D///B//+A//8Af/4AH/gAAYAA=="),e.x,e.y),
      remove:()=>{
        clearWatch(btnWatch);
        Bangle.removeListener("touch", touchHandler);
        g.reset().clearRect({x:WIDGETS.back.x, y:WIDGETS.back.y, w:24,h:24});
        delete WIDGETS.back;
        Bangle.drawWidgets();
      }
    }},global.WIDGETS);
    Bangle.drawWidgets();
  }
})
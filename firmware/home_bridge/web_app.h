// Haven web app: one page, all data through /api/* (web.h). Brand assets are vectors.
// Edit, then preview without the board: python3 tools/preview.py
#pragma once

// App icon: violet→coral tile, house roof + lightning bolt.
const char kIcon[] PROGMEM = R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64">
<defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#7C5CFF"/><stop offset="1" stop-color="#FF7A59"/></linearGradient></defs>
<rect width="64" height="64" rx="18" fill="url(#g)"/>
<path d="M15 31 32 17l17 14" fill="none" stroke="#fff" stroke-width="4.5" stroke-linecap="round" stroke-linejoin="round"/>
<path d="M35 26 25 39h7l-3 11 11-14h-7l2-10z" fill="#fff"/></svg>)svg";

// Link-preview card (Open Graph), 1200×630.
const char kOgImage[] PROGMEM = R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 630">
<defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#7C5CFF"/><stop offset="1" stop-color="#FF7A59"/></linearGradient>
<radialGradient id="r" cx=".8" cy="0" r="1"><stop offset="0" stop-color="#2a1f5c"/><stop offset="1" stop-color="#09090F"/></radialGradient></defs>
<rect width="1200" height="630" fill="url(#r)"/>
<g transform="translate(110 205) scale(3.5)"><rect width="64" height="64" rx="18" fill="url(#g)"/>
<path d="M15 31 32 17l17 14" fill="none" stroke="#fff" stroke-width="4.5" stroke-linecap="round" stroke-linejoin="round"/>
<path d="M35 26 25 39h7l-3 11 11-14h-7l2-10z" fill="#fff"/></g>
<text x="400" y="300" font-family="Segoe UI,Roboto,Helvetica,Arial,sans-serif" font-size="120" font-weight="800" fill="#F1F0FA">Haven</text>
<text x="404" y="380" font-family="Segoe UI,Roboto,Helvetica,Arial,sans-serif" font-size="40" fill="#9B99B8">Your Bluetooth home, on Wi-Fi and voice.</text></svg>)svg";

const char kManifest[] PROGMEM = R"json({"name":"Haven","short_name":"Haven","description":"Control your Bluetooth plugs and lights from anywhere in the house.",
"start_url":"/","display":"standalone","background_color":"#09090F","theme_color":"#09090F",
"icons":[{"src":"/icon.svg","sizes":"any","type":"image/svg+xml","purpose":"any maskable"}]})json";

const char kApp[] PROGMEM = R"html(<!doctype html><html lang=en><head><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>Haven</title>
<meta name=description content="Control your Bluetooth plugs and lights from anywhere in the house.">
<meta name=theme-color content="#09090F">
<meta name=color-scheme content=dark>
<meta name=apple-mobile-web-app-capable content=yes><meta name=mobile-web-app-capable content=yes>
<meta name=apple-mobile-web-app-status-bar-style content=black-translucent><meta name=apple-mobile-web-app-title content=Haven>
<meta property=og:type content=website><meta property=og:site_name content=Haven><meta property=og:title content="Haven">
<meta property=og:description content="Your Bluetooth home, on Wi-Fi and voice."><meta property=og:image content="/og.svg">
<meta name=twitter:card content=summary_large_image>
<link rel=icon type="image/svg+xml" href="/icon.svg"><link rel=apple-touch-icon href="/icon.svg"><link rel=manifest href="/manifest.webmanifest">
<style>
:root{--bg:#09090F;--card:#14141E;--card2:#1A1A27;--line:#26263A;--text:#F1F0FA;--mute:#9B99B8;--dim:#6B6988;
--brand:#7C5CFF;--accent:#FF7A59;--grad:linear-gradient(135deg,#7C5CFF,#FF7A59);--ok:#6CC8FF;--warn:#FFC24B;--bad:#FF5C7A;--r:24px}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}html{background:var(--bg)}
body{margin:0;font-family:ui-rounded,"SF Pro Rounded",system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;color:var(--text);min-height:100vh;
background:radial-gradient(700px 420px at 85% -120px,#2B1F63 0,transparent 70%),radial-gradient(600px 380px at -10% -60px,#3A1E2A 0,transparent 70%),var(--bg) fixed}
button{font:inherit;color:inherit;cursor:pointer}svg{display:block}
.wrap{max-width:480px;margin:0 auto;padding:0 16px calc(40px + env(safe-area-inset-bottom))}
/* splash */
#splash{position:fixed;inset:0;z-index:50;display:grid;place-items:center;background:var(--bg);transition:opacity .5s,visibility .5s}
#splash.out{opacity:0;visibility:hidden}.sp{display:grid;justify-items:center;gap:18px}
.sp-logo{position:relative;width:104px;height:104px;animation:pop .9s cubic-bezier(.2,1.4,.4,1) both}
.sp-logo svg{width:104px;height:104px;filter:drop-shadow(0 18px 40px #7C5CFF66)}
.ring{position:absolute;inset:-6px;border-radius:34px;border:2px solid #7C5CFF;animation:ring 1.8s ease-out .5s infinite;opacity:0}
.ring+.ring{animation-delay:1.1s;border-color:#FF7A59}
.word{display:flex;font-size:40px;font-weight:800;letter-spacing:.5px}
.word span{display:inline-block;background:var(--grad);-webkit-background-clip:text;background-clip:text;color:transparent;animation:rise .6s cubic-bezier(.2,.9,.3,1) both}
.tag{color:var(--mute);font-size:14px;animation:fade .6s 1s both}
@keyframes pop{0%{transform:scale(.3) rotate(-18deg);opacity:0}100%{transform:none;opacity:1}}
@keyframes ring{0%{transform:scale(.9);opacity:.8}100%{transform:scale(1.7);opacity:0}}
@keyframes rise{0%{transform:translateY(24px);opacity:0}100%{transform:none;opacity:1}}
@keyframes fade{from{opacity:0}to{opacity:1}}
/* app bar */
.bar{position:sticky;top:calc(10px + env(safe-area-inset-top));z-index:10;margin:10px 0 18px;display:flex;align-items:center;gap:12px;padding:10px 10px 10px 12px;
border-radius:22px;background:#14141ECC;backdrop-filter:blur(18px) saturate(1.4);-webkit-backdrop-filter:blur(18px) saturate(1.4);border:1px solid #ffffff12;box-shadow:0 10px 30px #0006}
.bar .logo{width:38px;height:38px;flex:none}.bar .logo svg{width:38px;height:38px}
.brand{flex:1;min-width:0}.brand b{display:block;font-size:19px;font-weight:800;background:var(--grad);-webkit-background-clip:text;background-clip:text;color:transparent}
.brand small{display:flex;align-items:center;gap:6px;font-size:12px;color:var(--mute);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.dot{width:7px;height:7px;border-radius:50%;background:var(--dim);flex:none}.dot.ok{background:var(--ok);box-shadow:0 0 8px var(--ok)}.dot.warn{background:var(--warn)}.dot.bad{background:var(--bad)}
.ib{width:42px;height:42px;border-radius:14px;border:1px solid var(--line);background:var(--card2);display:grid;place-items:center;flex:none;transition:.2s}
.ib:hover{border-color:#7C5CFF88}.ib:active{transform:scale(.94)}
.menu{position:absolute;right:8px;top:62px;min-width:210px;padding:6px;border-radius:18px;background:var(--card2);border:1px solid var(--line);box-shadow:0 20px 50px #000a;display:none}
.menu.open{display:block;animation:fade .15s}.menu button{display:flex;align-items:center;gap:12px;width:100%;padding:12px;border:0;background:none;border-radius:12px;text-align:left}
.menu button:hover{background:#ffffff0a}.menu .bad{color:var(--bad)}
/* cards */
.card{background:linear-gradient(180deg,var(--card2),var(--card));border:1px solid var(--line);border-radius:var(--r);padding:20px;margin-bottom:14px}
.row{display:flex;align-items:center;gap:12px}.sp1{flex:1;min-width:0}.mute{color:var(--mute)}.small{font-size:13px}
h2{font-size:24px;margin:4px 0 2px;font-weight:800}h3{font-size:16px;margin:0}
.hero h2 span{background:var(--grad);-webkit-background-clip:text;background-clip:text;color:transparent}
.pill{display:inline-flex;align-items:center;gap:6px;font-size:12px;font-weight:700;padding:6px 10px;border-radius:99px;background:#ffffff0d;color:var(--mute);white-space:nowrap}
.pill.ok{color:var(--ok);background:#6CC8FF1a}.pill.warn{color:var(--warn);background:#FFC24B1a}.pill.bad{color:var(--bad);background:#FF5C7A1a}
.dev{display:flex;align-items:center;gap:14px;cursor:pointer;transition:.2s}.dev:hover{border-color:#7C5CFF66}
.dicon{width:52px;height:52px;border-radius:17px;display:grid;place-items:center;background:#ffffff0a;color:var(--mute);flex:none;transition:.3s}
.dicon.on{background:var(--grad);color:#fff;box-shadow:0 10px 26px #7C5CFF55}
.sw{width:54px;height:32px;border-radius:99px;border:0;background:#2A2A3D;position:relative;flex:none;transition:.25s}
.sw:after{content:"";position:absolute;top:4px;left:4px;width:24px;height:24px;border-radius:50%;background:#fff;transition:.25s;box-shadow:0 2px 6px #0006}
.sw.on{background:var(--grad)}.sw.on:after{left:26px}.sw:disabled{opacity:.35;cursor:default}
.power{display:grid;place-items:center;margin:18px auto 4px;width:170px;height:170px;border-radius:50%;border:0;background:#1E1E2D;color:#5D5B7A;
box-shadow:inset 0 0 0 2px var(--line);transition:all .35s cubic-bezier(.2,.9,.3,1)}
.power.on{background:radial-gradient(circle at 35% 30%,#9B80FF,#7C5CFF 45%,#FF7A59);color:#fff;box-shadow:0 0 0 12px #7C5CFF1f,0 20px 70px #7C5CFF66}
.power:disabled{opacity:.35;cursor:default}.power:active:not(:disabled){transform:scale(.95)}
.big{text-align:center;font-size:32px;font-weight:800;margin-top:14px}.center{text-align:center}
.stats{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.stat{background:#ffffff08;border-radius:16px;padding:12px}
.stat b{display:block;font-size:20px;font-weight:800}.stat span{font-size:12px;color:var(--mute)}
.chips{display:flex;gap:8px;flex-wrap:wrap}.chip{padding:10px 14px;border-radius:13px;border:1px solid var(--line);background:#ffffff06;font-size:14px;font-weight:600}
.chip.on{background:var(--grad);border-color:transparent;color:#fff}
.chart{width:100%;height:150px;margin-top:10px}.axis{display:flex;justify-content:space-between;font-size:11px;color:var(--dim);margin-top:4px}
.rule{display:flex;align-items:center;gap:12px;padding:12px 0;border-top:1px solid var(--line)}.rule:first-of-type{border-top:0}
.rule b{font-size:20px;font-variant-numeric:tabular-nums}.days{font-size:12px;color:var(--mute);letter-spacing:.5px}
.sec{display:flex;align-items:center;justify-content:space-between;margin-bottom:12px}
.link{border:0;background:none;color:#A792FF;font-weight:700;padding:6px}
.kv{display:flex;justify-content:space-between;gap:12px;padding:13px 0;border-top:1px solid var(--line);font-size:15px}.kv:first-child{border-top:0}.kv span{color:var(--mute)}
.bars{display:flex;align-items:flex-end;gap:5px;height:44px}.bars i{width:10px;border-radius:4px;background:#2A2A3D}.bars i.on{background:var(--grad)}
label{display:block;font-size:13px;color:var(--mute);margin:14px 2px 7px}
.field{position:relative}.field input{padding-right:52px}
input[type=text],input[type=password],input[type=time]{width:100%;font:inherit;font-size:16px;padding:15px 16px;border-radius:15px;border:1px solid var(--line);background:#0E0E16;color:var(--text)}
input:focus{outline:2px solid var(--brand);border-color:transparent}
.eye{position:absolute;right:6px;top:50%;transform:translateY(-50%);width:42px;height:42px;border:0;background:none;color:var(--mute);display:grid;place-items:center;border-radius:12px}
.eye:hover{color:var(--text)}
.btn{width:100%;padding:16px;border-radius:16px;border:0;background:var(--grad);color:#fff;font-weight:800;margin-top:18px;box-shadow:0 12px 30px #7C5CFF44}
.btn:disabled{opacity:.5}.btn.ghost{background:transparent;border:1px solid var(--line);box-shadow:none;margin-top:10px}.btn.danger{background:#FF5C7A1f;color:var(--bad);box-shadow:none}
.err{color:var(--bad);font-size:14px;min-height:20px;margin-top:12px}.okmsg{color:var(--ok)}
.empty{text-align:center;padding:34px 20px}.empty .art{width:84px;height:84px;margin:0 auto 16px;border-radius:28px;display:grid;place-items:center;background:#7C5CFF1a;color:#A792FF}
.skel{height:86px;border-radius:var(--r);margin-bottom:14px;background:linear-gradient(90deg,#15151F 0,#1F1F2E 50%,#15151F 100%);background-size:200% 100%;animation:sh 1.2s infinite}
@keyframes sh{to{background-position:-200% 0}}
.net{display:flex;align-items:center;gap:12px;padding:13px 12px;border-radius:14px;cursor:pointer;border:1px solid transparent}
.net:hover,.net.sel{border-color:#7C5CFF88;background:#7C5CFF12}
.lock{width:72px;height:72px;border-radius:24px;margin:8px auto 16px;display:grid;place-items:center;background:#7C5CFF1a;color:#A792FF}
.swatches{display:flex;gap:10px;flex-wrap:wrap}.swatch{width:40px;height:40px;border-radius:50%;border:2px solid #ffffff22}
input[type=range]{width:100%;accent-color:#9B80FF}input[type=color]{width:40px;height:40px;border:0;padding:0;background:none}
#sheetbg{position:fixed;inset:0;z-index:30;background:#000a;opacity:0;visibility:hidden;transition:.25s}#sheetbg.open{opacity:1;visibility:visible}
#sheet{position:fixed;left:0;right:0;bottom:0;z-index:31;max-width:480px;margin:0 auto;padding:10px 20px calc(24px + env(safe-area-inset-bottom));
border-radius:28px 28px 0 0;background:var(--card2);border:1px solid var(--line);transform:translateY(105%);transition:transform .3s cubic-bezier(.2,.9,.3,1)}
#sheet.open{transform:none}#sheet .grab{width:40px;height:5px;border-radius:9px;background:var(--line);margin:0 auto 14px}
#toast{position:fixed;left:50%;bottom:calc(24px + env(safe-area-inset-bottom));z-index:40;transform:translate(-50%,140%);padding:13px 18px;border-radius:16px;
background:#232335;border:1px solid var(--line);font-weight:600;font-size:14px;transition:transform .3s;max-width:90vw;visibility:hidden}#toast.show{transform:translate(-50%,0);visibility:visible}
.back{display:flex;align-items:center;gap:6px;border:0;background:none;color:var(--mute);padding:4px 0 12px;font-weight:600}
.daybtn{width:40px;height:40px;border-radius:12px;border:1px solid var(--line);background:#ffffff06;font-weight:700}.daybtn.on{background:var(--grad);border-color:transparent;color:#fff}
.seg{display:flex;background:#0E0E16;border:1px solid var(--line);border-radius:14px;padding:4px}.seg button{flex:1;border:0;background:none;padding:10px;border-radius:10px;font-weight:700;color:var(--mute)}
.seg button.on{background:var(--card2);color:var(--text);box-shadow:0 2px 8px #0006}
</style></head><body>
<div id=splash><div class=sp><div class=sp-logo><i class=ring></i><i class=ring></i><svg id=splogo></svg></div>
<div class=word><span>H</span><span style="animation-delay:.08s">a</span><span style="animation-delay:.16s">v</span><span style="animation-delay:.24s">e</span><span style="animation-delay:.32s">n</span></div>
<div class=tag>Your Bluetooth home, on Wi-Fi.</div></div></div>
<div class=wrap>
<header class=bar id=bar><div class=logo id=barlogo></div><div class=brand><b>Haven</b><small><i class=dot id=bdot></i><span id=bsub>Connecting…</span></small></div>
<button class=ib id=bnet title=Network onclick="go('network')"></button><button class=ib id=bmenu title=Menu onclick="toggleMenu(event)"></button>
<div class=menu id=menu></div></header>
<main id=view></main></div>
<div id=sheetbg onclick=closeSheet()></div><div id=sheet></div><div id=toast></div>
<script>
const LOGO='<svg viewBox="0 0 64 64"><defs><linearGradient id="lg" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#7C5CFF"/><stop offset="1" stop-color="#FF7A59"/></linearGradient></defs><rect width="64" height="64" rx="18" fill="url(#lg)"/><path d="M15 31 32 17l17 14" fill="none" stroke="#fff" stroke-width="4.5" stroke-linecap="round" stroke-linejoin="round"/><path d="M35 26 25 39h7l-3 11 11-14h-7l2-10z" fill="#fff"/></svg>';
const P=(d,w=22)=>`<svg width=${w} height=${w} viewBox="0 0 24 24" fill=none stroke=currentColor stroke-width=2 stroke-linecap=round stroke-linejoin=round>${d}</svg>`;
const I={power:'<path d="M12 3v8M6.8 6.2a8 8 0 1 0 10.4 0"/>',plug:'<path d="M9 3v5M15 3v5M6 8h12v3a6 6 0 0 1-12 0zM12 17v4"/>',
bulb:'<path d="M9 18h6M10 21h4M12 3a6 6 0 0 0-4 10.5c.8.8 1 1.5 1 2.5h6c0-1 .2-1.7 1-2.5A6 6 0 0 0 12 3z"/>',
wifi:'<path d="M2 8.8a15 15 0 0 1 20 0M5.5 12.5a10 10 0 0 1 13 0M9 16.2a5 5 0 0 1 6 0"/><circle cx=12 cy=19.5 r=.6 fill=currentColor />',
wifioff:'<path d="M2 2l20 20M8.5 16.4a5 5 0 0 1 6.2-.6M5 12.6a10 10 0 0 1 4.3-2.4M16.7 10.9A10 10 0 0 1 19 12.5M2 8.8a15 15 0 0 1 4.4-2.7M10.7 5a15 15 0 0 1 11.3 3.8"/>',
dots:'<circle cx=5 cy=12 r=1.2 fill=currentColor /><circle cx=12 cy=12 r=1.2 fill=currentColor /><circle cx=19 cy=12 r=1.2 fill=currentColor />',
plus:'<path d="M12 5v14M5 12h14"/>',out:'<path d="M15 4h3a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2h-3M10 17l-5-5 5-5M5 12h11"/>',
back:'<path d="M15 18l-6-6 6-6"/>',gear:'<circle cx=12 cy=12 r=3 /><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1.1-1.5 1.7 1.7 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.5-1.1 1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.8.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8V9a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z"/>',
lock:'<rect x=4 y=11 width=16 height=10 rx=3 /><path d="M8 11V8a4 4 0 0 1 8 0v3"/>',eye:'<path d="M2 12s3.6-7 10-7 10 7 10 7-3.6 7-10 7S2 12 2 12z"/><circle cx=12 cy=12 r=3 />',
eyeoff:'<path d="M3 3l18 18M10.6 5.1A10 10 0 0 1 12 5c6.4 0 10 7 10 7a17 17 0 0 1-3 3.9M6.6 6.6A17 17 0 0 0 2 12s3.6 7 10 7a9.7 9.7 0 0 0 5.4-1.6M9.9 9.9a3 3 0 0 0 4.2 4.2"/>',
clock:'<circle cx=12 cy=12 r=9 /><path d="M12 7v5l3 2"/>',cal:'<rect x=3 y=5 width=18 height=16 rx=3 /><path d="M8 3v4M16 3v4M3 10h18"/>',
bolt:'<path d="M13 2 4 14h7l-1 8 9-12h-7z"/>',cloud:'<path d="M17.5 19a4.5 4.5 0 0 0 0-9h-1.3A7 7 0 1 0 6 17.3"/><path d="M6 19h11.5"/>',
search:'<circle cx=11 cy=11 r=7 /><path d="M20 20l-3.5-3.5"/>',trash:'<path d="M4 7h16M10 11v6M14 11v6M6 7l1 13h10l1-13M9 7V4h6v3"/>',bt:'<path d="M7 7l10 10-5 4V3l5 4L7 17"/>'};
const $=i=>document.getElementById(i),esc=s=>String(s??'').replace(/[&<>"']/g,c=>'&#'+c.charCodeAt(0)+';');
const DAYS=['M','T','W','T','F','S','S'],DAYN=['Mon','Tue','Wed','Thu','Fri','Sat','Sun'];
let S=null,view='home',devId=0,skew=0,started=Date.now(),hist={},unreachable=false,authMode='';
$('splogo').outerHTML=LOGO;$('barlogo').innerHTML=LOGO;$('bmenu').innerHTML=P(I.dots);

async function api(p,body){try{const r=await fetch(p,body?{method:'POST',body:new URLSearchParams(body)}:{});let j={};try{j=await r.json()}catch(e){}
unreachable=false;return{ok:r.ok,status:r.status,j}}catch(e){unreachable=true;return{ok:false,status:0,j:{error:'Can\'t reach Haven'}}}}
function toast(t){const e=$('toast');e.textContent=t;e.classList.add('show');clearTimeout(e._t);e._t=setTimeout(()=>e.classList.remove('show'),2600)}
function splashDone(){if(location.hash=='#splash')return;const w=Math.max(0,1500-(Date.now()-started));setTimeout(()=>$('splash').classList.add('out'),w)}
function go(v,id){const h='#'+v+(v=='device'?'/'+(id??devId):'');if(location.hash!=h&&v!='login')return location.hash=h;show(v,id)}
function show(v,id){view=v;if(id!=null)devId=+id;closeMenu();window.scrollTo(0,0);render(true);if(v=='device')loadHist();if(v=='wifi')scanWifi();if(v=='add')$('view')&&scanDevices()}
async function load(){const r=await api('/api/state');if(r.status==401){authMode=r.j.auth;view='login';render(true);return splashDone()}
if(r.ok){S=r.j;skew=S.system.time?S.system.time-Date.now()/1000:0;if(S.mode=='setup'&&!['wifi','login'].includes(view))go('wifi')}
render(false);splashDone()}
const now=()=>Date.now()/1000+skew,dev=()=>S&&S.devices[devId];
const fmtW=mw=>mw<0?'—':mw>=1e6?(mw/1e6).toFixed(2)+' kW':(mw/1000).toFixed(1)+' W',fmtWh=wh=>wh>=1000?(wh/1000).toFixed(2)+' kWh':wh.toFixed(wh<10?1:0)+' Wh';
const fmtDur=s=>{s=Math.max(0,Math.round(s));const d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);return d?d+'d '+h+'h':h?h+'h '+m+'m':m?m+'m':s+'s'};
const signal=r=>r>=-55?[4,'Excellent','ok']:r>=-65?[3,'Good','ok']:r>=-75?[2,'Fair','warn']:[1,'Weak','bad'];
const pad=n=>String(n).padStart(2,'0');
function render(force){
 // don't redraw under the user's fingers
 if(!force&&(document.activeElement&&['INPUT'].includes(document.activeElement.tagName)||$('sheet').classList.contains('open')))return;
 const loggedIn=view!='login';$('bar').style.visibility=loggedIn?'':'hidden';
 if(S&&loggedIn){const off=S.devices.filter(d=>!d.linked).length,sys=S.system;
  const st=unreachable?['bad','Can\'t reach Haven']:S.mode=='setup'?['warn','Setup mode']:off?['warn',off+' device'+(off>1?'s':'')+' out of reach']:
   [S.devices.length?'ok':'',S.devices.length?'All devices online':'No devices yet'];
  $('bdot').className='dot '+st[0];$('bsub').textContent=st[1];
  $('bnet').innerHTML=P(S.mode=='ok'?I.wifi:I.wifioff,20);$('bnet').style.color=S.mode=='ok'?['','var(--bad)','var(--warn)','var(--ok)','var(--ok)'][signal(sys.rssi)[0]]:'var(--warn)'}
 const v=$('view');
 if(view=='login')return v.innerHTML=loginView();
 if(!S)return v.innerHTML=unreachable?errorView():'<div class=skel></div><div class=skel style="height:200px"></div><div class=skel></div>';
 v.innerHTML=({home:homeView,device:deviceView,network:networkView,wifi:wifiView,add:addView})[view]();
 if(view=='device')drawChart()}
function errorView(){return`<div class="card empty"><div class=art>${P(I.wifioff,36)}</div><h2>Can't reach Haven</h2>
<p class=mute>Check that this phone is on the same Wi-Fi as the bridge, and that the bridge has power.</p><button class=btn onclick="load()">Try again</button></div>`}
function hello(){const h=new Date().getHours();return h<5?'Good night':h<12?'Good morning':h<18?'Good afternoon':'Good evening'}
function homeView(){const ds=S.devices,on=ds.filter(d=>d.on&&d.linked).length,w=ds.reduce((a,d)=>a+(d.kind=='plug'&&d.linked&&d.powerMw>0?d.powerMw:0),0);
 let h=`<section class="card hero"><div class="mute small">${hello()}</div><h2>${ds.length?`<span>${on}</span> of ${ds.length} on`:'Welcome home'}</h2>
 <div class="row mute small" style="margin-top:6px">${P(I.bolt,16)}<span>${fmtW(w)} right now</span></div></section>`;
 if(!ds.length)return h+`<section class="card empty"><div class=art>${P(I.bt,38)}</div><h3>Add your first device</h3>
 <p class="mute small">Haven finds AwoX plugs and Shada lamps nearby over Bluetooth.</p><button class=btn onclick="go('add')">Find devices</button></section>`;
 for(const d of ds){const ic=d.kind=='plug'?I.plug:I.bulb;const sub=!d.linked?`Searching…${d.reconnects?' · '+d.reconnects+' reconnects':''}`:
  d.kind=='plug'?(d.on?fmtW(d.powerMw):'Off')+(d.offAt?' · off in '+fmtDur(d.offAt-now()):''):(d.on?(d.white?'Warm white':'Colour')+' · '+d.brightness+'%':'Off');
  h+=`<section class="card dev" onclick="go('device',${d.id})"><div class="dicon ${d.on&&d.linked?'on':''}">${P(ic,26)}</div>
  <div class=sp1><h3>${esc(d.name)}</h3><div class="small ${d.linked?'mute':''}" style="${d.linked?'':'color:var(--warn)'}">${sub}</div></div>
  <button class="sw ${d.on&&d.linked?'on':''}" ${d.linked?'':'disabled'} onclick="event.stopPropagation();setDev(${d.id},{on:${d.on?0:1}})" aria-label="Toggle ${esc(d.name)}"></button></section>`}
 if(ds.length<S.system.maxDevices)h+=`<button class="btn ghost" onclick="go('add')"><span class=row style="justify-content:center">${P(I.plus,18)} Add device</span></button>`;
 return h}
function deviceView(){const d=dev();if(!d){view='home';return homeView()}
 const plug=d.kind=='plug',rules=S.rules.map((r,i)=>[r,i]).filter(([r])=>r.address==d.address);
 let h=`<button class=back onclick="go('home')">${P(I.back,18)} All devices</button>
 <section class=card><div class=row><div class=sp1><h2 style="margin:0">${esc(d.name)}</h2><div class="mute small">${plug?'Smart plug':'Colour lamp'} · ${esc(d.address.toUpperCase())}</div></div>
 <span class="pill ${d.linked?'ok':'warn'}"><i class="dot ${d.linked?'ok':'warn'}"></i>${d.linked?'Connected':'Searching…'}</span><button class=ib onclick="devSettings()">${P(I.gear,20)}</button></div>
 <button class="power ${d.on&&d.linked?'on':''}" ${d.linked?'':'disabled'} onclick="setDev(${d.id},{on:${d.on?0:1}})">${P(I.power,66)}</button>
 <div class=big>${!d.linked?'Out of reach':d.on?'On':'Off'}</div>
 <div class="center mute small">${!d.linked?`Make sure it has power and is within ~10 m.${d.reconnects?' Reconnected '+d.reconnects+'×.':''}`:'Connected for '+fmtDur(d.linkedFor)}</div></section>`;
 if(plug)h+=`<section class=card><div class=sec><h3>Energy</h3><span class="mute small">last 24 h</span></div>
 <div class=stats><div class=stat><b>${d.linked?fmtW(d.powerMw):'—'}</b><span>Now</span></div><div class=stat><b>${fmtWh(d.todayWh)}</b><span>Today</span></div><div class=stat><b>${fmtWh(d.yesterdayWh)}</b><span>Yesterday</span></div></div>
 <svg class=chart id=chart viewBox="0 0 300 150" preserveAspectRatio=none></svg><div class=axis><span>24 h ago</span><span>12 h</span><span>now</span></div></section>`;
 else h+=`<section class=card><h3>Colour</h3><div class=swatches style="margin:14px 0">${['ff7800','ff3b30','ff2d92','7c5cff','0a84ff','30d158','ffd60a'].map(c=>
 `<button class=swatch style="background:#${c}" onclick="setDev(${d.id},{color:'${c}'})" aria-label="#${c}"></button>`).join('')}<input type=color value="#${d.color}" onchange="setDev(${d.id},{color:this.value.slice(1)})"></div>
 <button class="chip ${d.white?'on':''}" onclick="setDev(${d.id},{white:1})">Warm white</button><label>Brightness · ${d.brightness}%</label>
 <input type=range min=5 max=100 value=${d.brightness} onchange="setDev(${d.id},{brightness:this.value})"></section>`;
 const left=d.offAt?d.offAt-now():0;
 h+=`<section class=card><div class=sec><h3>Sleep timer</h3>${left>0?`<span class="pill warn">${P(I.clock,14)} off in ${fmtDur(left)}</span>`:''}</div>
 <div class=chips>${[15,30,60,120,240].map(m=>`<button class=chip onclick="timer(${m})">${m<60?m+' min':m/60+' h'}</button>`).join('')}${left>0?'<button class=chip onclick="timer(0)">Cancel</button>':''}</div></section>
 <section class=card><div class=sec><h3>Schedules</h3><button class=link onclick="editRule(-1)">+ Add</button></div>`;
 if(!rules.length)h+=`<div class="mute small" style="padding:6px 0">${P(I.cal,18)}<p>No schedules yet. Turn ${esc(d.name)} on or off automatically at set times.</p></div>`;
 for(const[r,i]of rules)h+=`<div class=rule><div class=sp1 onclick="editRule(${i})" style="cursor:pointer"><b>${pad(r.hour)}:${pad(r.minute)}</b> <span class="pill ${r.on?'ok':''}">${r.on?'Turn on':'Turn off'}</span>
 <div class=days>${daysText(r.days)}</div></div><button class="sw ${r.enabled?'on':''}" onclick="toggleRule(${i})"></button></div>`;
 if(!S.system.time)h+=`<div class="small" style="color:var(--warn);margin-top:10px">Waiting for the clock to sync before schedules can run.</div>`;
 return h+'</section>'}
function daysText(m){if(m==127)return'Every day';if(m==31)return'Weekdays';if(m==96)return'Weekends';return DAYN.filter((_,i)=>m>>i&1).join(' · ')}
function networkView(){const s=S.system,[b,q,c]=signal(s.rssi),setup=S.mode=='setup';
 return`<button class=back onclick="go('home')">${P(I.back,18)} Home</button>
 <section class="card center"><div class=bars style="justify-content:center;margin:6px auto 12px">${[1,2,3,4].map(i=>`<i class="${!setup&&i<=b?'on':''}" style="height:${i*11}px"></i>`).join('')}</div>
 <h2>${setup?'Not connected':esc(s.ssid)}</h2><div class=mute>${setup?'Setup network “Haven-Setup” is open':`${q} signal · ${s.rssi} dBm`}</div>
 <span class="pill ${setup?'warn':c}" style="margin-top:12px">${setup?'Setup mode':'Connected'}</span></section>
 <section class=card><div class=kv><span>Address</span><b>${setup?esc(s.ip):'haven.local'}</b></div><div class=kv><span>IP address</span><b>${esc(s.ip)}</b></div>
 <div class=kv><span>Online for</span><b>${fmtDur(s.uptime)}</b></div><div class=kv><span>Wi-Fi reconnections</span><b>${s.reconnects}</b></div>
 <div class=kv><span>Clock</span><b>${s.time?'Synced':'Waiting…'}</b></div>
 <div class=kv><span>Alexa &amp; Google</span><b style="color:${!s.cloudConfigured?'var(--mute)':s.cloud?'var(--ok)':'var(--warn)'}">${!s.cloudConfigured?'Not set up':s.cloud?'Connected':'Connecting…'}</b></div></section>
 <button class=btn onclick="go('wifi')">Change Wi-Fi</button>`}
function eyeField(id,ph,ac){return`<div class=field><input type=password id=${id} placeholder="${ph}" autocomplete=${ac}><button type=button class=eye onclick="peek('${id}',this)" aria-label="Show password">${P(I.eye,20)}</button></div>`}
function peek(id,b){const e=$(id),show=e.type=='password';e.type=show?'text':'password';b.innerHTML=P(show?I.eyeoff:I.eye,20);b.setAttribute('aria-label',show?'Hide password':'Show password')}
function wifiView(){const setup=S&&S.mode=='setup';return`${setup?'':`<button class=back onclick="go('network')">${P(I.back,18)} Network</button>`}
 <section class=card><h2>Wi-Fi</h2><p class="mute small">Choose the network Haven should join.</p><div id=nets><div class=skel style="height:52px"></div><div class=skel style="height:52px"></div></div>
 <label>Network name</label><input type=text id=ssid autocomplete=off><label>Password</label>${eyeField('wpass','','off')}
 <button class=btn onclick="saveWifi()">Save &amp; restart</button><div class=err id=werr></div></section>`}
async function scanWifi(){const r=await api('/api/networks');const n=$('nets');if(!n)return;if(!r.ok||!r.j.length){n.innerHTML='<p class="mute small">No networks found. <button class=link onclick="scanWifi()">Scan again</button></p>';return}
 n.innerHTML=r.j.sort((a,b)=>b.rssi-a.rssi).map(x=>`<div class=net data-s="${esc(x.ssid)}" onclick="pickNet(this)">${P(I.wifi,20)}<span class=sp1>${esc(x.ssid)}</span><span class="mute small">${signal(x.rssi)[1]}</span>${x.secure?P(I.lock,16):''}</div>`).join('')}
function pickNet(e){document.querySelectorAll('.net').forEach(x=>x.classList.remove('sel'));e.classList.add('sel');$('ssid').value=e.dataset.s;$('wpass').focus()}
async function saveWifi(){const r=await api('/api/wifi',{ssid:$('ssid').value,pass:$('wpass').value});const e=$('werr');e.className=r.ok?'err okmsg':'err';
 e.textContent=r.ok?`Saved. Haven restarts and joins “${$('ssid').value}”. Reconnect this phone to that Wi-Fi, then open haven.local`:r.j.error}
function addView(){return`<button class=back onclick="go('home')">${P(I.back,18)} Home</button><section class=card><div class=sec><h2 style="margin:0">Add a device</h2><button class=link onclick="scanDevices()">Scan</button></div>
 <p class="mute small">Plug it in, close its phone app (it takes one connection at a time), and keep it within ~10 m.</p><div id=found><div class=skel style="height:64px"></div></div></section>`}
async function scanDevices(){const f=$('found');if(!f)return;f.innerHTML='<div class=skel style="height:64px"></div><p class="mute small center">Looking for devices… (8 s)</p>';
 const r=await api('/api/scan');if(!$('found'))return;if(!r.ok)return f.innerHTML=`<p class=err>${esc(r.j.error)}</p>`;
 if(!r.j.length)return f.innerHTML=`<div class=empty><div class=art>${P(I.search,34)}</div><h3>Nothing new nearby</h3><p class="mute small">Already-added devices aren't listed.</p><button class="btn ghost" onclick="scanDevices()">Scan again</button></div>`;
 window._found=r.j;f.innerHTML=r.j.map((x,i)=>`<div class=net onclick="addDev(${i})"><div class=dicon>${P(x.kind=='plug'?I.plug:I.bulb,24)}</div><div class=sp1><b>${esc(x.name)}</b><div class="mute small">${x.kind=='plug'?'Smart plug':'Lamp'} · ${signal(x.rssi)[1]} signal</div></div>${P(I.plus,20)}</div>`).join('')}
function addDev(i){const x=_found[i];sheet(`<h2>Name it</h2><p class="mute small">${x.kind=='plug'?'Smart plug':'Lamp'} · ${esc(x.address.toUpperCase())}</p>
 <label>Name</label><input type=text id=dname value="${esc(x.kind=='plug'?'Smart plug':'Lamp')}" maxlength=32><button class=btn id=dadd>Add device</button><div class=err id=derr></div>`);
 $('dadd').onclick=async()=>{const r=await api('/api/add',{address:x.address,type:x.type,kind:x.kind,name:$('dname').value});if(!r.ok)return $('derr').textContent=r.j.error;
  S=r.j;closeSheet();toast("Added — connecting…");go("home");if(x.kind)setTimeout(load,6000)}}
function devSettings(){const d=dev();sheet(`<h2>Device settings</h2><label>Name</label><input type=text id=rn value="${esc(d.name)}" maxlength=32>
 <label>Alexa / Google Home (Sinric Pro device ID)</label><input type=text id=rs value="${esc(d.sinric)}" placeholder="Optional" autocomplete=off>
 <button class=btn id=rsave>Save</button><button class="btn danger" id=rdel>${P(I.trash,18)} Remove device</button><div class=err id=rerr></div>`);
 $('rsave').onclick=async()=>{const restart=$('rs').value!=d.sinric;const r=await api('/api/rename',{id:d.id,name:$('rn').value,sinric:$('rs').value});
  if(!r.ok)return $('rerr').textContent=r.j.error;S=r.j;closeSheet();render(true);toast(restart?'Saved — Haven restarts to update Alexa':'Saved')};
 $('rdel').onclick=async()=>{if(!confirmBtn($('rdel'),'Tap again to remove'))return;await api('/api/remove',{id:d.id});closeSheet();toast('Removed — Haven restarts');view='home';S=null;render(true);setTimeout(load,6000)}}
function confirmBtn(b,t){if(b.dataset.armed)return true;b.dataset.armed=1;b.textContent=t;return false}
function editRule(i){const d=dev(),r=i>=0?S.rules[i]:{address:d.address,enabled:true,on:true,days:127,hour:7,minute:0};let days=r.days,on=r.on;
 sheet(`<h2>${i>=0?'Edit':'New'} schedule</h2><label>Time</label><input type=time id=rt value="${pad(r.hour)}:${pad(r.minute)}">
 <label>Action</label><div class=seg><button id=aon>Turn on</button><button id=aoff>Turn off</button></div>
 <label>Repeat</label><div class=chips id=rd>${DAYS.map((x,k)=>`<button class=daybtn data-k=${k}>${x}</button>`).join('')}</div>
 <button class=btn id=rsv>Save</button>${i>=0?'<button class="btn danger" id=rdl>Delete schedule</button>':''}`);
 const paint=()=>{$('aon').className=on?'on':'';$('aoff').className=on?'':'on';document.querySelectorAll('#rd button').forEach(b=>b.classList.toggle('on',days>>b.dataset.k&1))};paint();
 $('aon').onclick=()=>{on=true;paint()};$('aoff').onclick=()=>{on=false;paint()};document.querySelectorAll('#rd button').forEach(b=>b.onclick=()=>{days^=1<<b.dataset.k;paint()});
 $('rsv').onclick=()=>{if(!days)return toast('Pick at least one day');const[hh,mm]=$('rt').value.split(':').map(Number);const list=S.rules.slice(),nr={...r,days,on,hour:hh,minute:mm};
  if(i>=0)list[i]=nr;else list.push(nr);saveRules(list)};if(i>=0)$('rdl').onclick=()=>saveRules(S.rules.filter((_,k)=>k!=i))}
async function saveRules(list){const r=await api('/api/rules',{json:JSON.stringify(list)});if(!r.ok)return toast(r.j.error||'Could not save');S=r.j;closeSheet();render(true);toast('Schedule saved')}
function toggleRule(i){const list=S.rules.slice();list[i]={...list[i],enabled:!list[i].enabled};saveRules(list)}
async function timer(m){const r=await api('/api/timer',{id:devId,min:m});if(!r.ok)return toast(r.j.error);S=r.j;render(true);toast(m?'Turns off in '+fmtDur(m*60):'Timer cancelled')}
async function setDev(id,q){const d=S.devices[id];if(q.on!=null){d.on=!!q.on;render(true)}const r=await api('/api/device',{id,...q});if(r.ok){S=r.j;render(true)}else{toast(r.j.error||'Failed');load()}}
async function loadHist(){const d=dev();if(!d||d.kind!='plug')return;const r=await api('/api/history?id='+d.id);if(r.ok){hist[d.id]=r.j;drawChart()}}
function drawChart(){const c=$('chart'),h=hist[devId];if(!c)return;if(!h){c.innerHTML='';return}
 const w=h.w,max=Math.max(1,...w)*1.15,pts=w.map((v,i)=>[i*300/(w.length-1),150-v/max*140]);
 const line=pts.map((p,i)=>(i?'L':'M')+p[0].toFixed(1)+' '+p[1].toFixed(1)).join(' ');
 c.innerHTML=`<defs><linearGradient id=cg x1=0 y1=0 x2=1 y2=0><stop offset=0 stop-color="#7C5CFF"/><stop offset=1 stop-color="#FF7A59"/></linearGradient>
 <linearGradient id=cf x1=0 y1=0 x2=0 y2=1><stop offset=0 stop-color="#7C5CFF" stop-opacity=.35 /><stop offset=1 stop-color="#7C5CFF" stop-opacity=0 /></linearGradient></defs>
 <path d="${line} L300 150 L0 150Z" fill="url(#cf)"/><path d="${line}" fill=none stroke="url(#cg)" stroke-width=2.5 vector-effect=non-scaling-stroke stroke-linejoin=round />
 <text x=4 y=14 fill="#6B6988" font-size=11>${(max/1.15).toFixed(1)} W peak</text>`}
function loginView(){const c=authMode=='create';return`<section class=card style="margin-top:12vh"><div class=lock>${P(I.lock,30)}</div>
 <h2 class=center>${c?'Secure your Haven':'Welcome back'}</h2><p class="mute center small">${c?'Create a password for this bridge. At least 6 characters.':'Enter your Haven password.'}</p>
 <form onsubmit="return doLogin(event)"><label>Password</label>${eyeField('pw','',c?'new-password':'current-password')}
 ${c?'<label>Confirm password</label>'+eyeField('pw2','','new-password'):''}<button class=btn id=lbtn>${c?'Create & continue':'Sign in'}</button><div class=err id=lerr></div></form></section>`}
async function doLogin(e){e.preventDefault();if(authMode=='create'&&$('pw').value!=$('pw2').value){$('lerr').textContent='Passwords don\'t match';return false}
 $('lbtn').disabled=true;const r=await api('/api/login',{password:$('pw').value});$('lbtn').disabled=false;
 if(r.ok){view='home';load()}else $('lerr').textContent=r.j.error||'Sign-in failed';return false}
function toggleMenu(e){e.stopPropagation();const m=$('menu');m.innerHTML=`<button onclick="go('add')">${P(I.plus,20)} Add device</button><button onclick="go('network')">${P(I.wifi,20)} Network</button>
 <button class=bad onclick="logout()">${P(I.out,20)} Sign out</button>`;m.classList.toggle('open')}
function closeMenu(){$('menu').classList.remove('open')}document.addEventListener('click',closeMenu);
async function logout(){await api('/api/logout',{});S=null;authMode='login';view='login';render(true)}
function sheet(html){$('sheet').innerHTML='<div class=grab></div>'+html;$('sheet').classList.add('open');$('sheetbg').classList.add('open')}
function closeSheet(){$('sheet').classList.remove('open');$('sheetbg').classList.remove('open')}
{const[v,id]=location.hash.slice(1).split('/');if(['home','device','network','wifi','add'].includes(v)){view=v;if(id)devId=+id}}
addEventListener('hashchange',()=>{const[v,id]=(location.hash.slice(1)||'home').split('/');show(v,id)});
load().then(()=>{if(view=='device')loadHist();if(view=='wifi')scanWifi();if(view=='add')scanDevices()});setInterval(()=>{if(view!='login'&&view!='wifi'&&view!='add')load()},4000);setInterval(()=>view=='device'&&loadHist(),60000);
</script></body></html>)html";

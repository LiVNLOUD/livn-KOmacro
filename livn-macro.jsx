import { useState, useEffect, useRef } from "react";
const S=0.8,s=v=>v*S;

// ════════════════════════════════════════
//  LIVN MACRO v4.1
//  Hold/Toggle mode, No inject, UI Only
// ════════════════════════════════════════

// ─── Hold/Toggle Shortcut ───
function HotKey({label,val,set,mode,setMode,color="#8B5CF6"}){
  return(
    <div className="rounded flex items-center justify-between" style={{padding:`${s(4)}px ${s(6)}px`,marginBottom:s(3),background:"#0D111788",border:"1px solid #1F2937"}}>
      <span className="text-gray-300" style={{fontSize:s(10)}}>{label}</span>
      <div className="flex items-center" style={{gap:s(4)}}>
        <div className="flex rounded overflow-hidden" style={{border:`1px solid ${color}44`}}>
          <button onClick={()=>setMode("hold")} style={{fontSize:s(7.5),padding:`${s(1.5)}px ${s(6)}px`,background:mode==="hold"?color+"44":"#1F2937",color:mode==="hold"?color:"#6B7280"}}>Hold</button>
          <button onClick={()=>setMode("toggle")} style={{fontSize:s(7.5),padding:`${s(1.5)}px ${s(6)}px`,background:mode==="toggle"?color+"44":"#1F2937",color:mode==="toggle"?color:"#6B7280"}}>Toggle</button>
        </div>
        <input value={val} onChange={e=>set(e.target.value)} placeholder="Tuş" className="font-mono rounded border text-center focus:outline-none"
          style={{fontSize:s(9),width:s(48),padding:`${s(1.5)}px ${s(3)}px`,background:color+"22",color,borderColor:color+"44"}}/>
      </div>
    </div>
  );
}

// ─── Skill Slot ───
function SS({label,f,k,setF,setK,color="#8B5CF6",desc}){
  return(
    <div className="rounded" style={{padding:`${s(4)}px ${s(6)}px`,marginBottom:s(3),background:"#0D111788",border:"1px solid #1F2937"}}>
      <div className="flex items-center justify-between">
        <span className="text-gray-300" style={{fontSize:s(10),fontWeight:500}}>{label}</span>
        <div className="flex items-center" style={{gap:s(3)}}>
          <select value={f} onChange={e=>setF(e.target.value)} className="rounded border text-white font-mono cursor-pointer focus:outline-none"
            style={{fontSize:s(8.5),padding:`${s(1)}px ${s(3)}px`,background:"#1F2937",borderColor:color+"66",color,minWidth:s(38)}}>
            {["F1","F2","F3","F4","F5","F6","F7","F8"].map(x=><option key={x}>{x}</option>)}
          </select>
          <select value={k} onChange={e=>setK(e.target.value)} className="rounded border text-white font-mono cursor-pointer focus:outline-none"
            style={{fontSize:s(8.5),padding:`${s(1)}px ${s(3)}px`,background:"#1F2937",borderColor:color+"44",color:"#E5E7EB",minWidth:s(30)}}>
            {[..."0123456789"].map(x=><option key={x}>{x}</option>)}
          </select>
        </div>
      </div>
      {desc&&<div className="text-gray-500" style={{fontSize:s(8),marginTop:s(2)}}>{desc}</div>}
    </div>
  );
}

// ─── Dynamic Skill List ───
function DynSkills({label,skills,set,color="#8B5CF6",desc}){
  const add=()=>set([...skills,{f:"F1",k:"3"}]);
  const rm=i=>set(skills.filter((_,j)=>j!==i));
  const upd=(i,fld,v)=>{const n=[...skills];n[i]={...n[i],[fld]:v};set(n);};
  return(
    <div className="rounded" style={{padding:`${s(5)}px ${s(6)}px`,marginBottom:s(4),background:"#0D111788",border:"1px solid #1F2937"}}>
      <div className="flex items-center justify-between" style={{marginBottom:s(3)}}>
        <span className="text-gray-300 font-medium" style={{fontSize:s(10)}}>{label}</span>
        <button onClick={add} className="rounded font-mono" style={{fontSize:s(8),padding:`${s(1)}px ${s(6)}px`,background:color+"33",color,border:`1px solid ${color}44`}}>+ Ekle</button>
      </div>
      {desc&&<div className="text-gray-500" style={{fontSize:s(8),marginBottom:s(3)}}>{desc}</div>}
      {skills.map((sl,i)=>(
        <div key={i} className="flex items-center" style={{gap:s(3),marginBottom:s(2)}}>
          <span className="text-gray-500 font-mono" style={{fontSize:s(8),width:s(14)}}>{i+1}</span>
          <select value={sl.f} onChange={e=>upd(i,"f",e.target.value)} className="rounded border font-mono focus:outline-none"
            style={{fontSize:s(8.5),padding:`${s(1)}px ${s(2)}px`,background:"#1F2937",borderColor:color+"66",color,minWidth:s(36)}}>
            {["F1","F2","F3","F4","F5","F6","F7","F8"].map(x=><option key={x}>{x}</option>)}
          </select>
          <select value={sl.k} onChange={e=>upd(i,"k",e.target.value)} className="rounded border font-mono focus:outline-none"
            style={{fontSize:s(8.5),padding:`${s(1)}px ${s(2)}px`,background:"#1F2937",borderColor:color+"44",color:"#E5E7EB",minWidth:s(28)}}>
            {[..."0123456789"].map(x=><option key={x}>{x}</option>)}
          </select>
          <button onClick={()=>rm(i)} className="text-red-400 font-mono" style={{fontSize:s(9)}}>×</button>
        </div>
      ))}
      {skills.length===0&&<div className="text-gray-600" style={{fontSize:s(8.5)}}>Skill eklenmedi</div>}
    </div>
  );
}

// ─── Position Recorder ───
function PosRec({label,desc,sc,setSc,color="#F59E0B"}){
  return(
    <div className="rounded" style={{padding:`${s(5)}px ${s(6)}px`,marginBottom:s(4),background:"#0D111788",border:"1px solid #1F2937"}}>
      <div className="text-gray-300 font-medium" style={{fontSize:s(10),marginBottom:s(2)}}>{label}</div>
      {desc&&<div className="text-gray-500" style={{fontSize:s(8),marginBottom:s(3)}}>{desc}</div>}
      <div className="flex items-center" style={{gap:s(8)}}>
        <div className="flex items-center" style={{gap:s(3)}}>
          <span className="text-gray-400" style={{fontSize:s(9)}}>Kaydet Tuşu</span>
          <input value={sc} onChange={e=>setSc(e.target.value)} className="font-mono rounded border text-center focus:outline-none"
            style={{fontSize:s(9),width:s(48),padding:`${s(1.5)}px ${s(3)}px`,background:color+"22",color,borderColor:color+"44"}}/>
        </div>
      </div>
      <div className="text-gray-600" style={{fontSize:s(7.5),marginTop:s(2)}}>Mouse → konuma götür → tuşa bas → kaydedilir. Envanter kapalıysa I ile açılır.</div>
    </div>
  );
}

// ─── ROI Calib ───
function ROI({label,desc,color="#22D3EE",roi,setRoi}){
  return(
    <div className="rounded" style={{padding:`${s(5)}px ${s(6)}px`,marginBottom:s(4),background:"#0D111788",border:`1px solid ${color}33`}}>
      <div className="flex items-center justify-between">
        <div>
          <div className="text-gray-300 font-medium" style={{fontSize:s(10)}}>{label}</div>
          {desc&&<div className="text-gray-500" style={{fontSize:s(8),marginTop:1}}>{desc}</div>}
        </div>
        <button onClick={()=>setRoi(roi?null:{x:0,y:0,w:300,h:40})} className="rounded font-mono"
          style={{fontSize:s(8),padding:`${s(2)}px ${s(8)}px`,background:roi?color+"33":"#1F2937",color:roi?color:"#6B7280",border:`1px solid ${roi?color:"#374151"}`}}>
          {roi?"✓ Seçildi":"Seç"}
        </button>
      </div>
    </div>
  );
}

// ─── Base ───
function Toggle({on,set,label,sub}){return(
  <div className="flex items-center justify-between" style={{padding:`${s(4)}px 0`}}>
    <div><div className="text-gray-200" style={{fontSize:s(11.5),fontWeight:500}}>{label}</div>
    {sub&&<div className="text-gray-500" style={{fontSize:s(9),marginTop:1}}>{sub}</div>}</div>
    <button onClick={()=>set(!on)} className="relative rounded-full transition-all duration-300"
      style={{width:s(34),height:s(18),background:on?"#10B981":"#374151"}}>
      <div className="absolute bg-white rounded-full shadow transition-all duration-300"
        style={{width:s(14),height:s(14),top:s(2),left:on?s(18):s(2)}}/>
    </button>
  </div>
);}
function Sld({val,set,min=0,max=100,step=1,label,unit="ms",color="#8B5CF6"}){
  return(<div style={{padding:`${s(3)}px 0`}}>
    <div className="flex justify-between" style={{marginBottom:s(2)}}>
      <span className="text-gray-300" style={{fontSize:s(10.5)}}>{label}</span>
      <span className="font-mono rounded" style={{fontSize:s(8.5),padding:`${s(1)}px ${s(4)}px`,background:color+"33",color}}>{val}{unit}</span>
    </div>
    <input type="range" min={min} max={max} step={step} value={val} onChange={e=>set(+e.target.value)}
      className="w-full appearance-none cursor-pointer" style={{height:s(3.5),borderRadius:99,background:`linear-gradient(to right,${color} ${((val-min)/(max-min))*100}%,#1F2937 ${((val-min)/(max-min))*100}%)`}}/>
  </div>);
}
function Card({title,icon,accent="#8B5CF6",children,span}){return(
  <div className={span?"lg:col-span-2":""} style={{background:"#111827ee",border:"1px solid #1F2937",borderRadius:s(8),overflow:"hidden"}}>
    <div className="flex items-center border-b border-gray-800" style={{padding:`${s(6)}px ${s(9)}px`,gap:s(4)}}>
      {icon&&<span style={{fontSize:s(12)}}>{icon}</span>}
      <h3 className="font-semibold tracking-wide" style={{fontSize:s(10),color:accent}}>{title}</h3>
    </div>
    <div style={{padding:`${s(6)}px ${s(9)}px`}}>{children}</div>
  </div>
);}
function Info({children,type="info"}){
  const t={info:["#1E293B","#334155","#94A3B8","ℹ️"],success:["#022c22","#065f46","#6EE7B7","✅"],warn:["#422006","#92400e","#FCD34D","⚠️"],mech:["#1a1033","#4c1d95","#C4B5FD","⚙️"]}[type];
  return <div className="rounded" style={{fontSize:s(9),padding:s(6),background:t[0],border:`1px solid ${t[1]}`,color:t[2],marginTop:s(4)}}>{t[3]} {children}</div>;
}
function Combo({steps,color="#8B5CF6"}){return(
  <div className="flex items-center flex-wrap" style={{gap:s(2.5),padding:`${s(4)}px 0`}}>
    {steps.map((st,i)=>(<div key={i} className="flex items-center" style={{gap:s(2.5)}}>
      <div className="rounded font-mono font-bold text-center" style={{fontSize:s(8.5),padding:`${s(1.5)}px ${s(6)}px`,minWidth:s(24),
        background:st.h?color+"44":"#1F2937",border:`1px solid ${st.h?color:"#374151"}`,color:st.h?color:"#9CA3AF"}}>{st.l}</div>
      {i<steps.length-1&&<span style={{color:"#4B5563",fontSize:s(8)}}>→</span>}
    </div>))}
  </div>
);}
function HP({label,val,max,color,icon}){return(
  <div style={{marginBottom:s(4)}}>
    <div className="flex justify-between" style={{fontSize:s(8.5),marginBottom:s(2)}}>
      <span className="text-gray-400">{icon} {label}</span><span className="font-mono" style={{color}}>{val}/{max}</span>
    </div>
    <div className="rounded-full overflow-hidden" style={{height:s(4.5),background:"#1F2937"}}>
      <div className="h-full rounded-full" style={{width:Math.round(val/max*100)+"%",background:`linear-gradient(90deg,${color},${color}88)`}}/>
    </div>
  </div>
);}

// ════════ KALİBRASYON ════════
function CalibTab(){
  const [r1,s1]=useState(null);const[r2,s2]=useState(null);const[r3,s3]=useState(null);
  const[r4,s4]=useState(null);const[r5,s5]=useState(null);const[r6,s6]=useState(null);const[r7,s7]=useState(null);
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="ROI KALİBRASYON" icon="🎯" accent="#22D3EE" span>
      <Info type="warn">İlk kullanımda oyundaki UI alanlarını seçin. Mouse ile çerçeve çizilir, macro o bölgeyi izler.</Info>
      <div style={{marginTop:s(5)}}>
        <ROI label="Party Bar" desc="Parti HP barları" color="#22D3EE" roi={r1} setRoi={s1}/>
        <ROI label="Buff / Debuff Bar" desc="Aktif buff/debuff ikonları" color="#A855F7" roi={r2} setRoi={s2}/>
        <ROI label="HP Bar (Kendi)" desc="Kendi HP barı" color="#EF4444" roi={r3} setRoi={s3}/>
        <ROI label="MP Bar (Kendi)" desc="Kendi MP barı" color="#3B82F6" roi={r4} setRoi={s4}/>
        <ROI label="Skill Bar" desc="Aktif F tab skill ikonları" color="#F59E0B" roi={r5} setRoi={s5}/>
        <ROI label="Chat Alanı" desc="Chat penceresi (TP OCR)" color="#10B981" roi={r6} setRoi={s6}/>
        <ROI label="Silah Kırılma" desc="Silah kırılma ikonu alanı" color="#F472B6" roi={r7} setRoi={s7}/>
      </div>
      <Info type="mech">Buff tespiti: İkon parlaklık (aktif) → yanıp sönme ~500ms (bitmek üzere) → kaybolma (bitti). Frame karşılaştırma ile algılanır. Tüm karakterler için geçerlidir.</Info>
    </Card>
  </div>);
}

// ════════ MAGE ════════
function MageTab(){
  const[staff,setStaff]=useState(true);const[staffSC,setStaffSC]=useState("Caps");const[staffM,setStaffM]=useState("hold");
  const[nova,setNova]=useState(true);const[novaSC,setNovaSC]=useState("F9");const[novaM,setNovaM]=useState("hold");
  const[novaSkills,setNovaSkills]=useState([{f:"F1",k:"1"},{f:"F1",k:"2"},{f:"F1",k:"3"}]);
  const[vFarm,setVFarm]=useState(false);
  const[vfF,setVfF]=useState("F1");const[vfK,setVfK]=useState("4");
  const[tp,setTp]=useState(true);const[chatTp,setChatTp]=useState(true);const[tpTh,setTpTh]=useState(30);
  const[tpF,setTpF]=useState("F2");const[tpK,setTpK]=useState("5");const[tpOrd,setTpOrd]=useState("5,7,8");
  const[sDel,setSDel]=useState(300);const[nDel,setNDel]=useState(300);const[mob,setMob]=useState("Keilan");
  const[rSC,setRSC]=useState("F10");
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="STAFF COMBO" icon="⚡" accent="#8B5CF6">
      <Toggle on={staff} set={setStaff} label="Staff Combo" sub="Sekmeden staff ~300ms"/>
      <HotKey label="Shortcut" val={staffSC} set={setStaffSC} mode={staffM} setMode={setStaffM} color="#8B5CF6"/>
      <Sld val={sDel} set={setSDel} min={200} max={600} label="Staff Arası" unit="ms" color="#8B5CF6"/>
      <Combo color="#8B5CF6" steps={[{l:"Z",h:true},{l:`${sDel}ms`,h:false},{l:"Hit",h:false},{l:"...",h:false}]}/>
      <Info type="mech">Hold: tuş basılı = çalışır, bırak = durur. Toggle: bir bas = başla, tekrar bas = dur.</Info>
    </Card>
    <Card title="NOVA / AoE" icon="🔮" accent="#A78BFA">
      <Toggle on={nova} set={setNova} label="Nova Döngüsü" sub="AoE skill'leri sırayla"/>
      <HotKey label="Shortcut" val={novaSC} set={setNovaSC} mode={novaM} setMode={setNovaM} color="#A78BFA"/>
      <DynSkills label="AoE Skill Listesi" skills={novaSkills} set={setNovaSkills} color="#A78BFA" desc="İstediğiniz kadar ekleyin. Sırasıyla atılır."/>
      <Sld val={nDel} set={setNDel} min={150} max={600} label="Skill Arası" unit="ms" color="#A78BFA"/>
    </Card>
    <Card title="GÖRSEL FARM" icon="👁️" accent="#10B981">
      <Toggle on={vFarm} set={setVFarm} label="Görsel Farm" sub="Aktif olunca çalışır (genie tarzı spam)"/>
      <div style={{padding:`${s(3)}px 0`}}>
        <label className="text-gray-400 block" style={{fontSize:s(9),marginBottom:s(2)}}>Hedef Mob</label>
        <input value={mob} onChange={e=>setMob(e.target.value)} className="w-full border rounded text-white focus:outline-none"
          style={{fontSize:s(11),padding:`${s(3)}px ${s(7)}px`,background:"#1F2937",borderColor:"#374151"}} placeholder="Keilan"/>
      </div>
      <SS label="Spam Skill" f={vfF} k={vfK} setF={setVfF} setK={setVfK} color="#10B981" desc="Bu skill mob'a sürekli spam'lanır"/>
    </Card>
    <Card title="SMART TP" icon="🌀" accent="#22D3EE">
      <Toggle on={tp} set={setTp} label="HP TP"/><Toggle on={chatTp} set={setChatTp} label="Chat TP"/>
      <Sld val={tpTh} set={setTpTh} min={10} max={80} label="HP Eşiği" unit="%" color="#22D3EE"/>
      <SS label="TP Skill" f={tpF} k={tpK} setF={setTpF} setK={setTpK} color="#22D3EE"/>
      <div style={{padding:`${s(3)}px 0`}}>
        <label className="text-gray-400 block" style={{fontSize:s(9),marginBottom:s(2)}}>Sıralı TP</label>
        <input value={tpOrd} onChange={e=>setTpOrd(e.target.value)} className="w-full border rounded text-white font-mono focus:outline-none"
          style={{fontSize:s(11),padding:`${s(3)}px ${s(7)}px`,background:"#1F2937",borderColor:"#374151"}}/>
      </div>
    </Card>
    <Card title="RESTORE SİL" icon="🧹" accent="#F472B6">
      <PosRec label="Restore Pozisyonu" desc="Envanterde restore'un üzerine mouse → kaydet" sc={rSC} setSc={setRSC} color="#F472B6"/>
    </Card>
  </div>);
}

// ════════ PRIEST ════════
function PriestTab(){
  const[bp,setBp]=useState(false);const[bpSC,setBpSC]=useState("Caps");const[bpM,setBpM]=useState("hold");
  const[aF,setAF]=useState("F1");const[aK,setAK]=useState("2");const[aDel,setADel]=useState(300);
  const[kiF,setKiF]=useState("F2");const[kiK,setKiK]=useState("4");
  const[koF,setKoF]=useState("F2");const[koK,setKoK]=useState("5");
  const[heal,setHeal]=useState(true);
  const[hF,setHF]=useState("F1");const[hK,setHK]=useState("3");
  const[gF,setGF]=useState("F1");const[gK,setGK]=useState("4");
  const[hTh,setHTh]=useState(60);const[gTh,setGTh]=useState(40);
  const[buffs,setBuffs]=useState([{f:"F2",k:"1"},{f:"F2",k:"2"}]);
  const[cure,setCure]=useState(true);const[cF,setCF]=useState("F3");const[cK,setCK]=useState("1");
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="BP ATAK" icon="⚔️" accent="#F59E0B" span>
      <Toggle on={bp} set={setBp} label="BP Atak" sub="Skill→R→R + Kitap/Kol CD takibi (bağımsız)"/>
      <HotKey label="Atak Shortcut" val={bpSC} set={setBpSC} mode={bpM} setMode={setBpM} color="#F59E0B"/>
      <div className="grid grid-cols-1 md:grid-cols-3" style={{gap:s(8),marginTop:s(4)}}>
        <div>
          <div className="font-semibold" style={{fontSize:s(9.5),color:"#F59E0B",marginBottom:s(3)}}>🗡️ ATAK</div>
          <SS label="Skill" f={aF} k={aK} setF={setAF} setK={setAK} color="#F59E0B"/>
          <Sld val={aDel} set={setADel} min={150} max={500} label="Skill→R" unit="ms" color="#F59E0B"/>
        </div>
        <div>
          <div className="font-semibold" style={{fontSize:s(9.5),color:"#8B5CF6",marginBottom:s(3)}}>📖 KİTAP CD</div>
          <SS label="Kitap" f={kiF} k={kiK} setF={setKiF} setK={setKiK} color="#8B5CF6"/>
        </div>
        <div>
          <div className="font-semibold" style={{fontSize:s(9.5),color:"#22D3EE",marginBottom:s(3)}}>💪 KOL CD</div>
          <SS label="Kol" f={koF} k={koK} setF={setKoF} setK={setKoK} color="#22D3EE"/>
        </div>
      </div>
      <Info type="warn">Kitap/Kol: skill bar ikon pixel izleme ile CD takibi. Ataktan bağımsız paralel loop. Hold: basılı tutarken atak döner.</Info>
    </Card>
    <Card title="HEAL" icon="💚" accent="#22D3EE">
      <Toggle on={heal} set={setHeal} label="Akıllı Heal"/>
      <SS label="Tekli Heal" f={hF} k={hK} setF={setHF} setK={setHK} color="#22D3EE"/>
      <SS label="Toplu Heal" f={gF} k={gK} setF={setGF} setK={setGK} color="#06B6D4"/>
      <Sld val={hTh} set={setHTh} min={20} max={90} label="Tekli Eşik" unit="%" color="#22D3EE"/>
      <Sld val={gTh} set={setGTh} min={20} max={70} label="Toplu Eşik" unit="%" color="#06B6D4"/>
      <DynSkills label="Buff/AC" skills={buffs} set={setBuffs} color="#FBBF24" desc="İkon kaybolunca yenilenir"/>
    </Card>
    <Card title="CURE" icon="✨" accent="#34D399">
      <Toggle on={cure} set={setCure} label="Oto Cure"/>
      <SS label="Cure" f={cF} k={cK} setF={setCF} setK={setCK} color="#34D399"/>
      {[["#A855F7","Malice"],["#EF4444","Torment → Kalkan"],["#22C55E","Poison"],["#3B82F6","Slow"]].map(([c,n])=>(
        <div key={n} className="flex items-center" style={{gap:s(4),marginBottom:s(2),fontSize:s(9)}}>
          <div className="rounded-full" style={{width:s(7),height:s(7),background:c}}/><span style={{color:c,fontWeight:600}}>{n}</span>
        </div>
      ))}
    </Card>
  </div>);
}

// ════════ ROGUE ════════
function RogueTab(){
  const[vs,setVs]=useState(false);const[vsSC,setVsSC]=useState("Caps");const[vsM,setVsM]=useState("hold");
  const[mF,setMF]=useState("F1");const[mK,setMK]=useState("1");
  const[vsS,setVsS]=useState([{f:"F1",k:"3"},{f:"F1",k:"4"},{f:"F1",k:"5"}]);const[vsD,setVsD]=useState(300);
  const[sl,setSl]=useState(false);const[slSC,setSlSC]=useState("F9");const[slM,setSlM]=useState("hold");
  const[o3F,setO3F]=useState("F1");const[o3K,setO3K]=useState("3");
  const[o5F,setO5F]=useState("F1");const[o5K,setO5K]=useState("5");const[slD,setSlD]=useState(300);
  const[ok,setOk]=useState(false);const[okSC,setOkSC]=useState("F10");const[okM,setOkM]=useState("hold");
  const[okS,setOkS]=useState([{f:"F2",k:"5"},{f:"F2",k:"6"},{f:"F2",k:"7"}]);const[okD,setOkD]=useState(300);
  const[lf,setLf]=useState(true);const[lfF,setLfF]=useState("F2");const[lfK,setLfK]=useState("1");
  const[def,setDef]=useState(true);const[dfF,setDfF]=useState("F2");const[dfK,setDfK]=useState("3");
  const[otherS,setOtherS]=useState([{f:"F3",k:"2"},{f:"F3",k:"4"},{f:"F4",k:"1"},{f:"F3",k:"5"}]);
  const[s72,setS72]=useState(true);
  const[s7F,setS7F]=useState("F1");const[s7K,setS7K]=useState("7");
  const[s2F,setS2F]=useState("F1");const[s2K,setS2K]=useState("2");
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="VS / TOKAT" icon="⚔️" accent="#EF4444">
      <Toggle on={vs} set={setVs} label="VS Mode" sub="Minor→R→Skill→R"/>
      <HotKey label="Shortcut" val={vsSC} set={setVsSC} mode={vsM} setMode={setVsM} color="#EF4444"/>
      <SS label="Minor" f={mF} k={mK} setF={setMF} setK={setMK} color="#EF4444"/>
      <DynSkills label="Combo Skiller" skills={vsS} set={setVsS} color="#F87171" desc="Minor arasında sırayla"/>
      <Sld val={vsD} set={setVsD} min={150} max={500} label="Skill Arası" unit="ms" color="#EF4444"/>
      <Toggle on={lf} set={setLf} label="Oto Light Feet"/>
      <SS label="Light Feet" f={lfF} k={lfK} setF={setLfF} setK={setLfK} color="#FBBF24"/>
    </Card>
    <Card title="3-5 SLİDE" icon="🏹" accent="#F59E0B">
      <Toggle on={sl} set={setSl} label="3-5 Slide" sub="3'lü ok + W + 5'li ok + W"/>
      <HotKey label="Shortcut" val={slSC} set={setSlSC} mode={slM} setMode={setSlM} color="#F59E0B"/>
      <SS label="3'lü Ok" f={o3F} k={o3K} setF={setO3F} setK={setO3K} color="#F59E0B"/>
      <SS label="5'li Ok" f={o5F} k={o5K} setF={setO5F} setK={setO5K} color="#FBBF24"/>
      <Sld val={slD} set={setSlD} min={150} max={500} label="Skill Arası" unit="ms" color="#F59E0B"/>
      <Combo color="#F59E0B" steps={[{l:"3ok",h:true},{l:"W",h:false},{l:"5ok",h:true},{l:"W",h:false},{l:"...",h:false}]}/>
    </Card>
    <Card title="OKÇU / ICE+STYX+LN" icon="⚡" accent="#22D3EE">
      <Toggle on={ok} set={setOk} label="Sıralı Skill Combo" sub="Ice, Styx, Lightning vb."/>
      <HotKey label="Shortcut" val={okSC} set={setOkSC} mode={okM} setMode={setOkM} color="#22D3EE"/>
      <DynSkills label="Combo Sırası" skills={okS} set={setOkS} color="#22D3EE" desc="İstediğiniz sıra ve sayıda"/>
      <Sld val={okD} set={setOkD} min={150} max={600} label="Skill Arası" unit="ms" color="#22D3EE"/>
      <Toggle on={s72} set={setS72} label="72 Skill"/>
      <div className="grid grid-cols-2" style={{gap:s(3)}}>
        <SS label="7" f={s7F} k={s7K} setF={setS7F} setK={setS7K} color="#A855F7"/>
        <SS label="2" f={s2F} k={s2K} setF={setS2F} setK={setS2K} color="#7C3AED"/>
      </div>
    </Card>
    <Card title="OTO SKİLLER" icon="🤖" accent="#34D399">
      <Toggle on={def} set={setDef} label="Oto Def" sub="Buff ROI ikon izleme"/>
      <SS label="Defense" f={dfF} k={dfK} setF={setDfF} setK={setDfK} color="#FBBF24"/>
      <DynSkills label="Tek Tuş Skiller (Knife, M20, Cure, MS)" skills={otherS} set={setOtherS} color="#34D399" desc="Her biri kendi slot'unda"/>
    </Card>
  </div>);
}

// ════════ WARRIOR ════════
function WarriorTab(){
  const[des,setDes]=useState(true);const[dSC,setDSC]=useState("Caps");const[dM,setDM]=useState("toggle");
  const[dF,setDF]=useState("F1");const[dK,setDK]=useState("5");
  const[yv,setYv]=useState([{f:"F2",k:"3"},{f:"F2",k:"4"},{f:"F2",k:"5"},{f:"F2",k:"6"}]);
  const[dDel,setDDel]=useState(300);
  const[seri,setSeri]=useState(false);const[sSC,setSSC]=useState("F9");const[sM,setSM]=useState("hold");
  const[sF,setSF]=useState("F1");const[sK,setSK]=useState("3");const[sDel,setSDel]=useState(300);
  const[kalk,setKalk]=useState(true);const[kSC,setKSC]=useState("F10");const[kM,setKM]=useState("hold");
  const[fir,setFir]=useState(true);const[fSC,setFSC]=useState("Q");const[fM,setFM]=useState("toggle");
  const[db,setDb]=useState(true);
  const[kafF,setKafF]=useState("F3");const[kafK,setKafK]=useState("1");
  const[kilF,setKilF]=useState("F3");const[kilK,setKilK]=useState("2");
  const[kolF,setKolF]=useState("F3");const[kolK,setKolK]=useState("3");
  const[hbF,setHbF]=useState("F3");const[hbK,setHbK]=useState("4");
  const[kalPosSC,setKalPosSC]=useState("F12");
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="DES + YERE VUR" icon="💥" accent="#F59E0B">
      <Toggle on={des} set={setDes} label="Des Combo" sub="Des → F tab → Yere vur → Kalkan"/>
      <HotKey label="Shortcut" val={dSC} set={setDSC} mode={dM} setMode={setDM} color="#F59E0B"/>
      <SS label="Descent" f={dF} k={dK} setF={setDF} setK={setDK} color="#F59E0B"/>
      <DynSkills label="Yere Vur Skill'leri" skills={yv} set={setYv} color="#FBBF24" desc="Sırasıyla basılır"/>
      <Sld val={dDel} set={setDDel} min={200} max={600} label="Skill Arası" unit="ms" color="#FBBF24"/>
      <Info type="mech">Des → tab değiştir → yere vur sırasıyla ({dDel}ms) → kalkan. USKO optimal ~300ms.</Info>
    </Card>
    <Card title="BUFF CD TAKİP" icon="👑" accent="#EF4444">
      <Info type="info">Battle Cry = Kafa, Howling Sword = Kılıç. İkon kaybolunca yenilenir.</Info>
      <SS label="Kafa (Battle Cry)" f={kafF} k={kafK} setF={setKafF} setK={setKafK} color="#EF4444"/>
      <SS label="Kılıç (Howling Sword)" f={kilF} k={kilK} setF={setKilF} setK={setKilK} color="#F87171"/>
      <SS label="Kol" f={kolF} k={kolK} setF={setKolF} setK={setKolK} color="#FB923C"/>
      <SS label="HP Booster" f={hbF} k={hbK} setF={setHbF} setK={setHbK} color="#10B981"/>
    </Card>
    <Card title="SERİ ATAK" icon="🔥" accent="#F97316">
      <Toggle on={seri} set={setSeri} label="Seri Atak" sub="Tek skill + R"/>
      <HotKey label="Shortcut" val={sSC} set={setSSC} mode={sM} setMode={setSM} color="#F97316"/>
      <SS label="Atak Skill" f={sF} k={sK} setF={setSF} setK={setSK} color="#F97316"/>
      <Sld val={sDel} set={setSDel} min={150} max={500} label="Skill→R" unit="ms" color="#F97316"/>
      <Toggle on={kalk} set={setKalk} label="Kalkan Spam"/>
      <HotKey label="Kalkan Tuşu" val={kSC} set={setKSC} mode={kM} setMode={setKM} color="#FBBF24"/>
    </Card>
    <Card title="FIRFIR & DEBUFF & KALKAN" icon="🌀" accent="#A855F7">
      <Toggle on={fir} set={setFir} label="Fırfır"/>
      <HotKey label="Fırfır Tuşu" val={fSC} set={setFSC} mode={fM} setMode={setFM} color="#A855F7"/>
      <Toggle on={db} set={setDb} label="Debuff Sil" sub="Torment → kılıç sil → kalkan tak"/>
      <PosRec label="Kalkan Pozisyonu" desc="Envanterde kalkanın konumu" sc={kalPosSC} setSc={setKalPosSC} color="#F59E0B"/>
    </Card>
  </div>);
}

// ════════ EKSTRA ════════
function EkstraTab(){
  const[hp,setHp]=useState(true);
  const[hpF,setHpF]=useState("F1");const[hpK,setHpK]=useState("0");
  const[mpF,setMpF]=useState("F1");const[mpK,setMpK]=useState("9");
  const[hT,setHT]=useState(50);const[mT,setMT]=useState(30);
  const[farm,setFarm]=useState(false);const[fSC,setFSC]=useState("F11");const[fM,setFM]=useState("toggle");
  const[ham,setHam]=useState(true);const[hamF,setHamF]=useState("F4");const[hamK,setHamK]=useState("1");
  const[dur,setDur]=useState(false);const[durS,setDurS]=useState([{f:"F2",k:"1"}]);
  const[hud,setHud]=useState(true);
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="SMART HP/MP" icon="❤️" accent="#EF4444">
      <Toggle on={hp} set={setHp} label="Smart HP/MP"/>
      <HP label="HP" val={3240} max={5000} color="#EF4444" icon="❤️"/>
      <HP label="MP" val={4100} max={4500} color="#3B82F6" icon="💧"/>
      <SS label="HP Pot" f={hpF} k={hpK} setF={setHpF} setK={setHpK} color="#EF4444"/>
      <SS label="MP Pot" f={mpF} k={mpK} setF={setMpF} setK={setMpK} color="#3B82F6"/>
      <Sld val={hT} set={setHT} min={10} max={80} label="HP Eşik" unit="%" color="#EF4444"/>
      <Sld val={mT} set={setMT} min={10} max={80} label="MP Eşik" unit="%" color="#3B82F6"/>
    </Card>
    <Card title="OTO FARM" icon="🤖" accent="#10B981">
      <Toggle on={farm} set={setFarm} label="Oto Farm"/>
      <HotKey label="Shortcut" val={fSC} set={setFSC} mode={fM} setMode={setFM} color="#10B981"/>
      <Combo color="#10B981" steps={[{l:"Tab",h:true},{l:"HP?",h:false},{l:"Atak",h:true},{l:"HP=0?",h:false},{l:"Tab",h:false}]}/>
      <Toggle on={ham} set={setHam} label="Magic Hammer" sub="Kırılma ikonu ROI'da algılanır"/>
      <SS label="Hammer" f={hamF} k={hamK} setF={setHamF} setK={setHamK} color="#FBBF24"/>
    </Card>
    <Card title="DURATION" icon="⏱️" accent="#FBBF24">
      <Toggle on={dur} set={setDur} label="Oto Duration"/>
      <DynSkills label="Skill'ler" skills={durS} set={setDurS} color="#FBBF24" desc="İstediğiniz kadar. İkon kaybolunca yenilenir."/>
    </Card>
    <Card title="HUD" icon="🖥️" accent="#60A5FA">
      <Toggle on={hud} set={setHud} label="HUD Overlay"/>
      <div className="rounded-lg" style={{marginTop:s(4),padding:s(6),background:"#00000088",border:"1px solid #1F2937"}}>
        <div className="font-mono" style={{fontSize:s(7),color:"#10B981"}}>═══ LIVN HUD ═══</div>
        <div className="grid grid-cols-2" style={{gap:s(2),fontSize:s(7),fontFamily:"monospace",marginTop:s(2)}}>
          <span style={{color:"#EF4444"}}>HP: 64%</span><span style={{color:"#3B82F6"}}>MP: 91%</span>
          <span style={{color:"#10B981"}}>Driver OK</span><span style={{color:"#A78BFA"}}>Active</span>
        </div>
      </div>
    </Card>
  </div>);
}

// ════════ AYARLAR ════════
function AyarlarTab(){
  const[drv,setDrv]=useState(true);const[exe,setExe]=useState("KnightOnLine.exe");
  const[jit,setJit]=useState(5);const[hum,setHum]=useState(true);const[bg,setBg]=useState(false);
  const[koxp,setKoxp]=useState(false);
  return(<div className="grid grid-cols-1 lg:grid-cols-2" style={{gap:s(10)}}>
    <Card title="DRIVER" icon="🔒" accent="#10B981">
      <Toggle on={drv} set={setDrv} label="Interception Driver"/>
      <Toggle on={hum} set={setHum} label="Human-like"/>
      <Sld val={jit} set={setJit} min={0} max={30} label="Jitter" unit="±ms" color="#10B981"/>
      <Toggle on={bg} set={setBg} label="Background Input"/>
    </Card>
    <Card title="SİSTEM" icon="⚙️" accent="#60A5FA">
      <div style={{padding:`${s(3)}px 0`}}>
        <label className="text-gray-400 block" style={{fontSize:s(9),marginBottom:s(2)}}>Hedef EXE</label>
        <input value={exe} onChange={e=>setExe(e.target.value)} className="w-full border rounded text-white font-mono focus:outline-none"
          style={{fontSize:s(11),padding:`${s(3)}px ${s(7)}px`,background:"#1F2937",borderColor:"#374151"}}/>
      </div>
    </Card>
    <Card title="PROJE DURUMU" icon="📋" accent="#FBBF24" span>
      <Info type="warn">Bu proje şu an <strong>konfigürasyon arayüzüdür</strong>. Gerçek macro backend'i henüz yazılmadı. Inject yoktur. Oyun dosyalarına, hafızasına dokunmaz.</Info>
      <div style={{marginTop:s(6)}}>
        <div className="font-semibold" style={{fontSize:s(10),color:"#FBBF24",marginBottom:s(4)}}>🛣️ UYGULAMA YOLU</div>
        <div style={{fontSize:s(9)}}>
          {[
            ["1. Electron + React","Bu UI'ı Electron ile sarmalayarak masaüstü uygulaması yap","#10B981","Kolay"],
            ["2. Python Prototip","PyAutoGUI + Pillow (screen cap) + Tesseract OCR + keyboard lib","#22D3EE","Orta"],
            ["3. C++ Backend","Win32 BitBlt/DXGI + Tesseract + Interception Driver","#8B5CF6","Zor"],
          ].map(([t,d,c,diff])=>(
            <div key={t} className="rounded" style={{padding:s(7),marginBottom:s(4),background:"#0D1117",border:"1px solid #1F2937"}}>
              <div className="flex items-center justify-between">
                <span className="font-semibold" style={{color:c}}>{t}</span>
                <span className="font-mono rounded" style={{fontSize:s(8),padding:`${s(1)}px ${s(6)}px`,background:c+"22",color:c}}>{diff}</span>
              </div>
              <div className="text-gray-400" style={{fontSize:s(8.5),marginTop:s(2)}}>{d}</div>
            </div>
          ))}
        </div>
        <Info type="success">En hızlı deneme yolu: <strong>Python prototip</strong>. PyAutoGUI ile tuş basma, Pillow ile ekran yakalama, pytesseract ile OCR. Windows'ta çalışır, Interception driver olmadan SendInput kullanır. Sonra C++'a geçiş yapılır.</Info>
        <Info type="mech">
          <strong>Koxp seçeneği:</strong> Ayarlar sekmesinde Koxp Modu toggle'ı var ama implementasyonu yok. Bu, DLL inject + memory read ile çalışan ayrı bir proje olacak. Macro modundan bağımsız, isteğe bağlı geçiş.
        </Info>
      </div>
    </Card>
  </div>);
}

// ════════ DASHBOARD ════════
function DashboardTab(){
  const logs=[
    {t:"14:32:01",m:"[MAGE] Staff hit (hold aktif)",c:"#34D399"},
    {t:"14:32:03",m:"[OCR] Mob: Keilan ✓",c:"#60A5FA"},
    {t:"14:32:05",m:"[TP] HP %28 → TP",c:"#A78BFA"},
    {t:"14:32:08",m:"[PRIEST] Kitap ikon parladı → bas",c:"#22D3EE"},
    {t:"14:32:10",m:"[CURE] Malice → Cure",c:"#34D399"},
    {t:"14:32:13",m:"[WAR] Kafa ikonu kayboldu → yenilendi",c:"#FBBF24"},
  ];
  return(<div style={{display:"flex",flexDirection:"column",gap:s(10)}}>
    <div className="grid grid-cols-1 lg:grid-cols-3" style={{gap:s(10)}}>
      <Card title="DURUM" icon="📊" accent="#10B981">
        <HP label="HP" val={3240} max={5000} color="#EF4444" icon="❤️"/>
        <HP label="MP" val={4100} max={4500} color="#3B82F6" icon="💧"/>
      </Card>
      <div className="lg:col-span-2">
        <Card title="LOG" icon="📜" accent="#60A5FA">
          <div style={{maxHeight:s(120),overflowY:"auto",scrollbarWidth:"thin"}}>
            {logs.map((l,i)=><div key={i} className="flex font-mono" style={{gap:s(4),fontSize:s(8),padding:`${s(1)}px 0`}}>
              <span className="text-gray-600 shrink-0">{l.t}</span><span style={{color:l.c}}>{l.m}</span>
            </div>)}
          </div>
        </Card>
      </div>
    </div>
    <Info type="warn">İlk kullanımda <strong>Kalibrasyon</strong> sekmesinden ROI alanlarını seçin. <strong>Ayarlar</strong> sekmesinde proje durumu ve deneme yol haritası mevcut.</Info>
  </div>);
}

// ════════ MAIN ════════
const TABS=["Dashboard","Kalibrasyon","Mage","Priest","Rogue","Warrior","Ekstra","Ayarlar"];
const ICONS={Dashboard:"📊",Kalibrasyon:"🎯",Mage:"🔮",Priest:"✝️",Rogue:"🗡️",Warrior:"🛡️",Ekstra:"🧩",Ayarlar:"⚙️"};

export default function LivnMacro(){
  const[tab,setTab]=useState("Dashboard");const[run,setRun]=useState(false);const[clk,setClk]=useState("00:00:00");const ref=useRef(null);
  useEffect(()=>{if(!run){setClk("00:00:00");ref.current=null;return;}ref.current=Date.now();
    const iv=setInterval(()=>{if(!ref.current)return;const d=Date.now()-ref.current;
      setClk([Math.floor(d/3600000),Math.floor((d%3600000)/60000),Math.floor((d%60000)/1000)].map(v=>String(v).padStart(2,"0")).join(":"));
    },1000);return()=>clearInterval(iv);},[run]);
  const P={Dashboard:<DashboardTab/>,Kalibrasyon:<CalibTab/>,Mage:<MageTab/>,Priest:<PriestTab/>,Rogue:<RogueTab/>,Warrior:<WarriorTab/>,Ekstra:<EkstraTab/>,Ayarlar:<AyarlarTab/>};
  return(
    <div className="min-h-screen text-white" style={{background:"linear-gradient(160deg,#06060a,#0a0e17,#0d0a14)",fontFamily:"'Segoe UI',system-ui,sans-serif",fontSize:s(14)}}>
      <div className="fixed inset-0 pointer-events-none" style={{opacity:0.03,backgroundImage:"radial-gradient(circle at 1px 1px,#fff 0.5px,transparent 0)",backgroundSize:`${s(32)}px ${s(32)}px`}}/>
      <div className="relative mx-auto" style={{maxWidth:s(1200),padding:`${s(14)}px ${s(12)}px`}}>
        <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between" style={{marginBottom:s(14),gap:s(8)}}>
          <div>
            <h1 className="font-bold" style={{fontSize:s(19)}}>
              <span style={{background:"linear-gradient(135deg,#8B5CF6,#22D3EE,#10B981)",WebkitBackgroundClip:"text",WebkitTextFillColor:"transparent"}}>LIVN MACRO</span>
              <span className="text-gray-600 font-light" style={{fontSize:s(10),marginLeft:s(5)}}>v4.1</span>
            </h1>
            <p className="text-gray-600" style={{fontSize:s(8),marginTop:s(2)}}>Ekran Okuma • ROI Kalibrasyon • Hold/Toggle • No Inject</p>
          </div>
          <div className="flex items-center" style={{gap:s(6)}}>
            <div className="font-mono text-gray-500 rounded-lg border border-gray-800" style={{fontSize:s(8.5),padding:`${s(3)}px ${s(7)}px`,background:"#0D1117"}}>⏱ {clk}</div>
            <button onClick={()=>setRun(!run)} className="rounded-lg font-semibold transition-all"
              style={{fontSize:s(9.5),padding:`${s(5)}px ${s(12)}px`,background:run?"linear-gradient(135deg,#EF4444,#DC2626)":"linear-gradient(135deg,#10B981,#059669)"}}>
              {run?"⏹ DURDUR":"▶ BAŞLAT"}
            </button>
          </div>
        </div>
        <div className="flex overflow-x-auto" style={{gap:s(1.5),marginBottom:s(12),scrollbarWidth:"none"}}>
          {TABS.map(t=>(
            <button key={t} onClick={()=>setTab(t)} className="whitespace-nowrap transition-all"
              style={{fontSize:s(9.5),padding:`${s(4.5)}px ${s(10)}px`,borderRadius:s(5),background:tab===t?"#1F293788":"transparent",borderBottom:`2px solid ${tab===t?"#8B5CF6":"transparent"}`,color:tab===t?"#E5E7EB":"#6B7280"}}>
              <span style={{marginRight:s(3)}}>{ICONS[t]}</span>{t}
            </button>
          ))}
        </div>
        <div style={{paddingBottom:s(16)}}>{P[tab]}</div>
        <div className="text-center text-gray-800" style={{fontSize:s(7.5),paddingBottom:s(8)}}>Livn Macro — No inject • Ekranı görür, anlar, uygular</div>
      </div>
    </div>
  );
}

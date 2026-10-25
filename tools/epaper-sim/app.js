(() => {
  const TAGS = ["note", "work", "idea", "buy", "private"];
  const LANGS = ["tr", "en", "de"];
  const IDLE = [30, 45, 90, 0];

  const DEVICE = {
    en: {
      today: "TODAY",
      mon: ["JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"],
      menu: ["Notes", "Tags", "Sync", "Settings"],
      headerSettings: "settings",
      headerRecord: "record",
      headerTag: "tag",
      headerSync: "sync",
      headerSetup: "setup",
      notesTitle: "notes",
      tagsAll: "tags: all",
      tagsPref: "tags: ",
      metricNotes: "NOTES",
      metricPending: "PENDING",
      metricTasks: "TASKS",
      metricMarks: "MARKS",
      metricHint: "HINT",
      metricWifi: "Wi-Fi",
      metricStatus: "STATUS",
      metricAction: "ACTION",
      on: "ON",
      off: "OFF",
      wifiConnect: "PWR connect",
      wifiDisconnect: "PWR disconnect",
      bootStop: "BOOT stop",
      empty: "(empty)",
      deleteQ: "Delete?",
      deleteHint: "PWR yes / BOOT no",
      playing: "Playing...",
      holdBootMenu: "hold BOOT menu",
      holdPwrDel: "hold PWR del",
      pwrPlay: "PWR play",
      bootNext: "BOOT next",
      setup2: "Open captive",
      setup3: "portal",
      soundOn: "Sound ON",
      soundOff: "Sound OFF",
      langPref: "Lang ",
      idleOff: "Idle OFF",
      idlePref: "Idle ",
      idleUnit: "s",
      orgOn: "Org ON",
      orgOff: "Org OFF",
      sdFail: "SD fail",
      noApiKey: "No API key",
      tag: ["note", "work", "idea", "buy", "private"],
      st: { pending: "pending", ready: "ready", error: "error", recording: "recording", transcribing: "transcribing" },
    },
    de: {
      today: "HEUTE",
      mon: ["JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"],
      menu: ["Notizen", "Tags", "Sync", "Einstell."],
      headerSettings: "einstell.",
      headerRecord: "aufnahme",
      headerTag: "tag",
      headerSync: "sync",
      headerSetup: "setup",
      notesTitle: "notizen",
      tagsAll: "tags: alle",
      tagsPref: "tags: ",
      metricNotes: "NOTIZEN",
      metricPending: "OFFEN",
      metricTasks: "AUFGABEN",
      metricMarks: "MARKEN",
      metricHint: "HINWEIS",
      metricWifi: "WLAN",
      metricStatus: "STATUS",
      metricAction: "AKTION",
      on: "AN",
      off: "AUS",
      wifiConnect: "PWR verbinden",
      wifiDisconnect: "PWR trennen",
      bootStop: "BOOT stop",
      empty: "(leer)",
      deleteQ: "Loschen?",
      deleteHint: "PWR ja / BOOT nein",
      playing: "Spielt...",
      holdBootMenu: "BOOT halten Menu",
      holdPwrDel: "PWR halten del",
      pwrPlay: "PWR play",
      bootNext: "BOOT weiter",
      setup2: "Portal im",
      setup3: "Browser",
      soundOn: "Ton AN",
      soundOff: "Ton AUS",
      langPref: "Sprache ",
      idleOff: "Idle AUS",
      idlePref: "Idle ",
      idleUnit: "s",
      orgOn: "Org AN",
      orgOff: "Org AUS",
      sdFail: "SD Fehler",
      noApiKey: "Kein API-Key",
      tag: ["notiz", "arbeit", "idee", "kauf", "privat"],
      st: { pending: "offen", ready: "fertig", error: "fehler", recording: "aufnahme", transcribing: "schrift" },
    },
    tr: {
      today: "BUGUN",
      mon: ["OCA", "SUB", "MAR", "NIS", "MAY", "HAZ", "TEM", "AGU", "EYL", "EKI", "KAS", "ARA"],
      menu: ["Notlar", "Etiketler", "Senkron", "Ayarlar"],
      headerSettings: "ayarlar",
      headerRecord: "kayit",
      headerTag: "etiket",
      headerSync: "senkron",
      headerSetup: "kurulum",
      notesTitle: "notlar",
      tagsAll: "etiket: tumu",
      tagsPref: "etiket: ",
      metricNotes: "NOTLAR",
      metricPending: "BEKLEYEN",
      metricTasks: "GOREVLER",
      metricMarks: "ISARET",
      metricHint: "IPUCU",
      metricWifi: "Wi-Fi",
      metricStatus: "DURUM",
      metricAction: "EYLEM",
      on: "ACIK",
      off: "KAPALI",
      wifiConnect: "PWR baglan",
      wifiDisconnect: "PWR baglantiyi kes",
      bootStop: "BOOT durdur",
      empty: "(bos)",
      deleteQ: "Silinsin mi?",
      deleteHint: "PWR evet / BOOT hayir",
      playing: "Oynatiliyor...",
      holdBootMenu: "BOOT basili menu",
      holdPwrDel: "PWR basili sil",
      pwrPlay: "PWR oynat",
      bootNext: "BOOT sonraki",
      setup2: "Tarayicida",
      setup3: "portal ac",
      soundOn: "Ses ACIK",
      soundOff: "Ses KAPALI",
      langPref: "Dil ",
      idleOff: "Bekleme KAPALI",
      idlePref: "Bekleme ",
      idleUnit: "sn",
      orgOn: "Duzen ACIK",
      orgOff: "Duzen KAPALI",
      sdFail: "SD hata",
      noApiKey: "API anahtari yok",
      tag: ["not", "is", "fikir", "al", "ozel"],
      st: { pending: "bekliyor", ready: "hazir", error: "hata", recording: "kayit", transcribing: "yaziliyor" },
    },
  };

  function dui() {
    return DEVICE[lang] || DEVICE.tr;
  }

  function tagLabelUi(tag) {
    const i = TAGS.indexOf(tag);
    return dui().tag[i >= 0 ? i : 0];
  }

  function statusLabelUi(status) {
    return dui().st[status] || status;
  }

  const strings = {
    tr: { idle: "INote", recording: "Kaydediliyor…", setupAp: "Wi-Fi kur", notes: "not", wifiOk: "Wi-Fi", wifiNo: "Wi-Fi yok", mark: "+işaret" },
    en: { idle: "INote", recording: "Recording…", setupAp: "Setup Wi-Fi", notes: "notes", wifiOk: "Wi-Fi", wifiNo: "No Wi-Fi", mark: "+mark" },
    de: { idle: "INote", recording: "Aufnahme…", setupAp: "WLAN einrichten", notes: "Notizen", wifiOk: "WLAN", wifiNo: "Kein WLAN", mark: "+Mark" },
  };

  /** @type {'home'|'record'|'tag'|'menu'|'notes'|'tags'|'sync'|'settings'|'setup'|'sleep'} */
  let state = "home";
  let menuIndex = 0;
  let settingsIndex = 0;
  let notesIndex = 0;
  let tagsFilter = 0; // 0 = all
  let tagsNoteIndex = 0;
  let confirmDelete = false;
  let pendingTag = "note";
  let markerCount = 0;
  let markFlash = false;
  let recordingSec = 0;
  let recordTimer = null;
  let playing = false;
  let wifiConnected = false;
  let syncHint = "";
  let soundEnabled = true;
  let lang = "tr";
  let idleSleep = 45;
  let organize = false;
  let battery = 76;

  let notes = [
    { id: "a1", displayId: "#001", tag: "work", status: "ready" },
    { id: "b2", displayId: "#002", tag: "buy", status: "pending" },
    { id: "c3", displayId: "#003", tag: "idea", status: "error", error: "STT timeout" },
  ];

  const canvas = document.getElementById("panel");
  const ctx = canvas.getContext("2d");
  const stateLabel = document.getElementById("stateLabel");
  const wakeRow = document.getElementById("wakeRow");
  const sdOkEl = document.getElementById("sdOk");
  const apiKeyEl = document.getElementById("hasApiKey");

  function s() {
    return strings[lang] || strings.en;
  }

  function sdOk() {
    return sdOkEl.checked;
  }

  function hasApiKey() {
    return apiKeyEl.checked;
  }

  function pendingCount() {
    return notes.filter((n) => n.status === "pending").length;
  }

  function errorCount() {
    return notes.filter((n) => n.status === "error").length;
  }

  function filteredTagNotes() {
    if (tagsFilter === 0) return notes.slice();
    const t = TAGS[tagsFilter - 1];
    return notes.filter((n) => n.tag === t);
  }

  function settingsLabels() {
    const d = dui();
    return [
      soundEnabled ? d.soundOn : d.soundOff,
      d.langPref + lang,
      idleSleep === 0 ? d.idleOff : d.idlePref + idleSleep + d.idleUnit,
      organize ? d.orgOn : d.orgOff,
    ];
  }

  function homeStatus() {
    const d = dui();
    if (!sdOk()) return d.sdFail;
    if (!hasApiKey()) return d.noApiKey;
    return "";
  }

  function syncStatus() {
    return syncHint || homeStatus();
  }

  // Panel approximates device FreeSans Pala-like chrome (DisplayUi.cpp)
  const FONT = '"IBM Plex Sans"';
  const PANEL = "#f7f4ec";
  const INK = "#111111";
  const WHITE = "#f7f4ec";
  const MX = 10;
  const RULE_Y = 32;
  const CHIP_H = 28;
  const CHIP_GAP = 4;
  const CHIP_R = 5;
  const CHIP_Y0 = 40;
  const TAG_CHIP_H = 24;
  const TAG_CHIP_GAP = 2;

  function clear() {
    ctx.fillStyle = PANEL;
    ctx.fillRect(0, 0, 200, 200);
    ctx.fillStyle = INK;
    ctx.strokeStyle = INK;
    ctx.lineWidth = 1.25;
  }

  function roundRect(x, y, w, h, r, fill) {
    ctx.beginPath();
    ctx.moveTo(x + r, y);
    ctx.arcTo(x + w, y, x + w, y + h, r);
    ctx.arcTo(x + w, y + h, x, y + h, r);
    ctx.arcTo(x, y + h, x, y, r);
    ctx.arcTo(x, y, x + w, y, r);
    ctx.closePath();
    if (fill) ctx.fill();
    else ctx.stroke();
  }

  function drawWifiArc(cx, cy, r) {
    ctx.beginPath();
    ctx.arc(cx, cy, r, Math.PI * 1.15, Math.PI * 1.85);
    ctx.stroke();
  }

  function drawWifiIcon(x, y, connected) {
    const cx = x + 9;
    const cy = y + 11;
    ctx.strokeStyle = INK;
    ctx.fillStyle = INK;
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.arc(cx, cy, 1.5, 0, Math.PI * 2);
    ctx.fill();
    if (connected) {
      drawWifiArc(cx, cy, 4);
      drawWifiArc(cx, cy, 7);
      drawWifiArc(cx, cy, 10);
    } else {
      drawWifiArc(cx, cy, 7);
      ctx.beginPath();
      ctx.moveTo(cx - 5, cy - 8);
      ctx.lineTo(cx + 5, cy - 2);
      ctx.stroke();
    }
    ctx.lineWidth = 1.25;
  }

  function drawBatteryIcon(x, y, pct) {
    ctx.strokeStyle = INK;
    ctx.fillStyle = INK;
    roundRect(x, y, 24, 12, 2, false);
    roundRect(x + 24, y + 3, 3, 6, 1, true);
    const level = Math.max(0, Math.min(100, pct));
    let fillW = Math.round((level * 20) / 100);
    if (fillW <= 0 && level > 0) fillW = 1;
    if (fillW > 0) roundRect(x + 2, y + 2, fillW, 8, 1, true);
  }

  function headerBar(titleLower) {
    ctx.fillStyle = INK;
    ctx.font = "700 13px " + FONT;
    ctx.fillText(String(titleLower).slice(0, 18), MX, 22);
    ctx.beginPath();
    ctx.moveTo(MX, RULE_Y + 0.5);
    ctx.lineTo(200 - MX, RULE_Y + 0.5);
    ctx.stroke();
    drawWifiIcon(136, 3, wifiConnected);
    drawBatteryIcon(160, 4, battery);
  }

  function headerBrand() {
    headerBar("INote");
  }

  function chipAt(y, h, label, selected) {
    const w = 200 - 2 * MX;
    ctx.strokeStyle = INK;
    ctx.fillStyle = INK;
    if (selected) {
      roundRect(MX, y, w, h, CHIP_R, true);
      ctx.fillStyle = WHITE;
    } else {
      roundRect(MX, y, w, h, CHIP_R, false);
      ctx.fillStyle = INK;
    }
    ctx.font = "700 12px " + FONT;
    ctx.fillText(String(label).slice(0, 16), MX + 10, y + h - 7);
    ctx.fillStyle = INK;
  }

  function menuChip(index, label, selected) {
    chipAt(CHIP_Y0 + index * (CHIP_H + CHIP_GAP), CHIP_H, label, selected);
  }

  function tagChip(index, label, selected) {
    chipAt(CHIP_Y0 + index * (TAG_CHIP_H + TAG_CHIP_GAP), TAG_CHIP_H, label, selected);
  }

  function metricRow(y, label, value) {
    ctx.strokeStyle = INK;
    ctx.beginPath();
    ctx.moveTo(MX, y + 0.5);
    ctx.lineTo(200 - MX, y + 0.5);
    ctx.stroke();
    ctx.fillStyle = INK;
    ctx.font = "700 11px " + FONT;
    ctx.fillText(label, MX, y + 16);
    const v = String(value);
    const tw = ctx.measureText(v).width;
    ctx.fillText(v, 200 - MX - tw, y + 16);
  }

  function contentBox(y, h) {
    ctx.strokeStyle = INK;
    roundRect(MX, y, 200 - 2 * MX, h, CHIP_R, false);
  }

  function bodyLine(y, t) {
    ctx.fillStyle = INK;
    ctx.font = "700 11px " + FONT;
    ctx.fillText(String(t).slice(0, 20), MX, y);
  }

  function bodyLineIn(x, y, t) {
    ctx.fillStyle = INK;
    ctx.font = "700 11px " + FONT;
    ctx.fillText(String(t).slice(0, 18), x, y);
  }

  function footerHint(t) {
    bodyLine(192, t);
  }

  function paint() {
    const ui = dui();
    clear();
    if (state === "sleep") {
      ctx.fillStyle = INK;
      ctx.font = "700 28px " + FONT;
      const label = "INote";
      const tw = ctx.measureText(label).width;
      ctx.fillText(label, (200 - tw) / 2, 112);
      wakeRow.classList.remove("hidden");
      stateLabel.textContent = "state: sleep";
      return;
    }
    wakeRow.classList.add("hidden");

    if (state === "home") {
      headerBrand();
      const d = new Date();
      const date = String(d.getDate()).padStart(2, "0") + " " + ui.mon[d.getMonth()];
      bodyLine(52, date);
      ctx.font = "700 26px " + FONT;
      ctx.fillStyle = INK;
      ctx.fillText(ui.today, MX, 82);
      metricRow(100, ui.metricNotes, notes.length);
      metricRow(128, ui.metricPending, pendingCount());
      metricRow(156, ui.metricTasks, notes.filter((n) => n.tag === "buy" || n.status === "ready").length > 0 ? 2 : 0);
    } else if (state === "menu") {
      headerBar("menu");
      for (let i = 0; i < 4; i++) menuChip(i, ui.menu[i], i === menuIndex);
    } else if (state === "settings") {
      headerBar(ui.headerSettings);
      const labs = settingsLabels();
      for (let i = 0; i < 4; i++) menuChip(i, labs[i], i === settingsIndex);
    } else if (state === "record") {
      headerBar(ui.headerRecord);
      ctx.font = "700 26px " + FONT;
      ctx.fillStyle = INK;
      ctx.fillText(recordingSec + " " + ui.idleUnit, MX, 78);
      metricRow(100, ui.metricMarks, markerCount);
      if (markFlash) metricRow(128, ui.metricMarks, "+");
      footerHint(ui.bootStop);
    } else if (state === "tag") {
      headerBar(ui.headerTag);
      const sel = Math.max(0, TAGS.indexOf(pendingTag));
      for (let i = 0; i < TAGS.length; i++) tagChip(i, ui.tag[i], i === sel);
    } else if (state === "notes" || state === "tags") {
      const list = state === "notes" ? notes : filteredTagNotes();
      const idx = state === "notes" ? notesIndex : tagsNoteIndex;
      const ht =
        state === "notes"
          ? ui.notesTitle
          : tagsFilter === 0
            ? ui.tagsAll
            : ui.tagsPref + ui.tag[tagsFilter - 1];
      headerBar(ht);
      contentBox(40, 72);
      if (!list.length) {
        bodyLineIn(MX + 8, 62, ui.empty);
      } else {
        const n = list[Math.min(idx, list.length - 1)];
        bodyLineIn(MX + 8, 62, n.displayId);
        bodyLineIn(
          MX + 8,
          84,
          n.status === "error" && n.error
            ? n.error.slice(0, 18)
            : tagLabelUi(n.tag) + " · " + statusLabelUi(n.status)
        );
      }
      if (confirmDelete && state === "notes") {
        contentBox(120, 56);
        bodyLineIn(MX + 8, 142, ui.deleteQ);
        bodyLineIn(MX + 8, 164, ui.deleteHint);
      } else {
        metricRow(
          124,
          ui.metricHint,
          playing ? ui.playing : state === "notes" ? ui.holdBootMenu : ui.pwrPlay
        );
        footerHint(state === "notes" ? ui.holdPwrDel : ui.pwrPlay);
      }
    } else if (state === "sync") {
      headerBar(ui.headerSync);
      const st = syncStatus();
      if (wifiConnected) {
        metricRow(48, ui.metricWifi, ui.on);
        metricRow(76, "SSID", "DemoNet");
        metricRow(104, "IP", "192.168.1.42");
        if (st) metricRow(132, ui.metricStatus, st.slice(0, 12));
        footerHint(ui.wifiDisconnect);
      } else {
        metricRow(48, ui.metricWifi, ui.off);
        metricRow(76, ui.metricAction, ui.wifiConnect);
        if (st) metricRow(104, ui.metricStatus, st.slice(0, 12));
        if (pendingCount() > 0) metricRow(132, ui.metricPending, pendingCount());
      }
    } else if (state === "setup") {
      headerBar(ui.headerSetup);
      contentBox(40, 80);
      bodyLineIn(MX + 8, 68, "INote-Setup");
      bodyLineIn(MX + 8, 92, ui.setup2);
      bodyLineIn(MX + 8, 112, ui.setup3);
    }

    stateLabel.textContent = "state: " + state + (confirmDelete ? " (del?)" : "");
  }

  function stopRecordTimer() {
    if (recordTimer) {
      clearInterval(recordTimer);
      recordTimer = null;
    }
  }

  function startRecord() {
    if (!sdOk()) {
      paint();
      return;
    }
    state = "record";
    markerCount = 0;
    markFlash = false;
    recordingSec = 0;
    stopRecordTimer();
    recordTimer = setInterval(() => {
      recordingSec += 1;
      paint();
    }, 1000);
    paint();
  }

  function stopToTag() {
    stopRecordTimer();
    pendingTag = "note";
    state = "tag";
    paint();
  }

  function enterSleep() {
    stopRecordTimer();
    playing = false;
    confirmDelete = false;
    state = "sleep";
    paint();
  }

  function wake() {
    state = "home";
    paint();
  }

  function bootShort() {
    if (state === "sleep") {
      wake();
      return;
    }
    if (state === "home") startRecord();
    else if (state === "record") stopToTag();
    else if (state === "tag") {
      const i = TAGS.indexOf(pendingTag);
      pendingTag = TAGS[(i + 1) % TAGS.length];
      paint();
    } else if (state === "menu") {
      menuIndex = (menuIndex + 1) % 4;
      paint();
    } else if (state === "notes") {
      if (confirmDelete) {
        confirmDelete = false;
        paint();
        return;
      }
      if (notes.length) {
        notesIndex = (notesIndex + 1) % notes.length;
        paint();
      }
    } else if (state === "tags") {
      const list = filteredTagNotes();
      if (!list.length || tagsNoteIndex + 1 >= list.length) {
        tagsFilter = (tagsFilter + 1) % (TAGS.length + 1);
        tagsNoteIndex = 0;
      } else tagsNoteIndex++;
      paint();
    } else if (state === "settings") {
      settingsIndex = (settingsIndex + 1) % 4;
      paint();
    } else if (state === "setup") {
      /* SoftAP — stay */
      paint();
    }
  }

  function bootHold1() {
    if (state === "sleep") {
      wake();
      return;
    }
    if (state === "home") {
      state = "menu";
      menuIndex = 0;
      paint();
    } else if (state === "record") {
      markerCount += 1;
      markFlash = true;
      paint();
      setTimeout(() => {
        markFlash = false;
        paint();
      }, 800);
    } else if (state === "menu") {
      state = "home";
      paint();
    } else if (state === "notes") {
      playing = false;
      confirmDelete = false;
      state = "menu";
      paint();
    } else if (state === "tags" || state === "sync" || state === "settings") {
      playing = false;
      state = "menu";
      paint();
    }
  }

  function bootHold3() {
    if (state === "sleep") {
      wake();
      return;
    }
    if (state === "record") return;
    stopRecordTimer();
    playing = false;
    confirmDelete = false;
    state = "setup";
    paint();
  }

  function pwrShort() {
    if (state === "sleep") {
      wake();
      return;
    }
    if (state === "home") {
      enterSleep();
    } else if (state === "tag") {
      // confirm tag — fake new note if coming from record
      notes.unshift({
        id: "n" + Date.now(),
        displayId: "#" + String(notes.length + 1).padStart(3, "0"),
        tag: pendingTag,
        status: "pending",
      });
      state = "home";
      paint();
    } else if (state === "menu") {
      if (menuIndex === 0) {
        state = "notes";
        notesIndex = 0;
        confirmDelete = false;
      } else if (menuIndex === 1) {
        state = "tags";
        tagsFilter = 0;
        tagsNoteIndex = 0;
      } else if (menuIndex === 2) {
        state = "sync";
        syncHint = "";
      } else {
        state = "settings";
        settingsIndex = 0;
      }
      paint();
    } else if (state === "notes") {
      if (confirmDelete) {
        if (notes.length) {
          notes.splice(notesIndex, 1);
          if (notesIndex >= notes.length) notesIndex = Math.max(0, notes.length - 1);
        }
        confirmDelete = false;
        playing = false;
        paint();
        return;
      }
      if (!soundEnabled || !notes.length) {
        paint();
        return;
      }
      playing = !playing;
      paint();
    } else if (state === "tags") {
      const list = filteredTagNotes();
      if (!soundEnabled || !list.length) {
        paint();
        return;
      }
      playing = !playing;
      paint();
    } else if (state === "sync") {
      if (wifiConnected) {
        wifiConnected = false;
        syncHint = "";
      } else {
        // demo: fail once if no api key + no wifi name — always succeed for flow, or fail if unchecked sd
        wifiConnected = true;
        syncHint = "";
      }
      paint();
    } else if (state === "settings") {
      if (settingsIndex === 0) {
        soundEnabled = !soundEnabled;
        if (!soundEnabled) playing = false;
      } else if (settingsIndex === 1) {
        lang = LANGS[(LANGS.indexOf(lang) + 1) % LANGS.length];
      } else if (settingsIndex === 2) {
        idleSleep = IDLE[(IDLE.indexOf(idleSleep) + 1 + IDLE.length) % IDLE.length];
        if (IDLE.indexOf(idleSleep) < 0) idleSleep = 30;
      } else organize = !organize;
      paint();
    }
  }

  function pwrLong() {
    if (state === "sleep") {
      wake();
      return;
    }
    if (state === "home") {
      // soft power-off → sleep face (sim)
      enterSleep();
    } else if (state === "notes" && !confirmDelete && notes.length) {
      confirmDelete = true;
      paint();
    }
  }

  document.querySelectorAll("[data-act]").forEach((btn) => {
    btn.addEventListener("click", () => {
      const a = btn.getAttribute("data-act");
      if (a === "boot-short") bootShort();
      else if (a === "boot-hold1") bootHold1();
      else if (a === "boot-hold3") bootHold3();
      else if (a === "pwr-short") pwrShort();
      else if (a === "pwr-long") pwrLong();
      else if (a === "wake") wake();
    });
  });

  sdOkEl.addEventListener("change", () => paint());
  apiKeyEl.addEventListener("change", () => paint());

  // Gap note: Sync always succeeds in sim; real device may show Wi-Fi fail / No SSID.
  if (document.fonts && document.fonts.ready) {
    document.fonts.ready.then(() => paint());
  } else {
    paint();
  }
})();

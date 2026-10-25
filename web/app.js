(() => {
  const F = window.INoteFormat;
  const listEl = document.getElementById("noteList");
  const taskListEl = document.getElementById("taskList");
  const emptyEl = document.getElementById("empty");
  const searchEl = document.getElementById("search");
  const statusEl = document.getElementById("deviceStatus");
  const player = document.getElementById("player");
  const doneRow = document.getElementById("doneRow");
  const doneCb = document.getElementById("detailDone");
  const markerList = document.getElementById("markerList");

  let notes = [];
  let typeFilter = "all";
  let tagFilter = "";
  let taskFilter = "open";
  let query = "";
  let fixtureMode = false;
  let activeNote = null;
  let features = { ask: false, briefing: false };
  let calCursor = new Date();
  calCursor.setDate(1);
  let selectedDay = "";

  const statusLabel = {
    ready: "hazır",
    pending: "bekleyen",
    transcribing: "işleniyor",
    recording: "kayıt",
    error: "hata",
  };
  const typeLabel = { note: "not", task: "görev", appointment: "randevu" };

  function escapeHtml(s) {
    return String(s)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function displayLabel(n) {
    return n.displayId || n.id;
  }

  function previewText(n) {
    if (n.title) return n.title;
    if (n.transcript) return n.transcript;
    if (n.status === "pending") return "Transkripsiyon bekleniyor…";
    if (n.status === "error") return n.error || "Hata";
    return "(metin yok)";
  }

  function noteMatchesQuery(n) {
    const q = query.trim().toLocaleLowerCase("tr");
    if (!q) return true;
    const hay = `${n.transcript || ""} ${n.title || ""} ${n.id} ${n.displayId || ""} ${n.tag || ""} ${n.error || ""}`.toLocaleLowerCase(
      "tr"
    );
    return hay.includes(q);
  }

  function filteredNotes() {
    return notes.filter((n) => {
      if (typeFilter === "pending" || typeFilter === "error") {
        if (n.status !== typeFilter) return false;
      }
      if (tagFilter && (n.tag || "note") !== tagFilter) return false;
      return noteMatchesQuery(n);
    });
  }

  function taskNotes() {
    return notes
      .filter((n) => n.type === "task")
      .filter((n) => {
        if (taskFilter === "open") return !n.done;
        if (taskFilter === "done") return !!n.done;
        return true;
      })
      .filter(noteMatchesQuery)
      .sort((a, b) => {
        const da = a.dueAt || "9999";
        const db = b.dueAt || "9999";
        if (da !== db) return da < db ? -1 : 1;
        return (a.createdAt || "") < (b.createdAt || "") ? 1 : -1;
      });
  }

  function fmtDuration(ms) {
    const s = Math.round((ms || 0) / 1000);
    if (s < 60) return `${s} sn`;
    return `${Math.floor(s / 60)}:${String(s % 60).padStart(2, "0")}`;
  }

  function renderNoteButton(n, list) {
    const li = document.createElement("li");
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "note";
    const tag = n.tag || "note";
    btn.innerHTML = `
      <div class="note-head">
        <span><span class="note-id">${escapeHtml(displayLabel(n))}</span><span class="note-tag">${escapeHtml(tag)}</span> · ${fmtDuration(n.durationMs)}</span>
        <span class="status ${n.status}">${statusLabel[n.status] || n.status}</span>
      </div>
      <p class="preview">${escapeHtml(previewText(n))}</p>`;
    btn.addEventListener("click", () => openPlayer(n));
    li.appendChild(btn);
    list.appendChild(li);
  }

  function renderNotes() {
    const items = filteredNotes();
    listEl.innerHTML = "";
    emptyEl.classList.toggle("hidden", items.length > 0);
    items.forEach((n, i) => {
      renderNoteButton(n, listEl);
      listEl.lastElementChild.querySelector(".note").style.animationDelay = `${Math.min(i, 12) * 30}ms`;
    });
  }

  function renderTasks() {
    taskListEl.innerHTML = "";
    taskNotes().forEach((n) => renderNoteButton(n, taskListEl));
  }

  function notesForDay(day) {
    return notes.filter((n) => {
      if (n.dueAt && F.dayKey(n.dueAt) === day) return true;
      if (!n.dueAt && n.createdAt && F.dayKey(n.createdAt) === day) return true;
      return false;
    });
  }

  function daysWithItems(year, month) {
    const set = new Set();
    for (const n of notes) {
      const iso = n.dueAt || n.createdAt;
      if (!iso) continue;
      const d = new Date(iso);
      if (d.getFullYear() === year && d.getMonth() === month) set.add(F.dayKey(iso));
    }
    return set;
  }

  function renderCalendar() {
    const y = calCursor.getFullYear();
    const m = calCursor.getMonth();
    document.getElementById("calTitle").textContent = `${String(m + 1).padStart(2, "0")}.${y}`;
    const grid = document.getElementById("calGrid");
    grid.innerHTML = "";
    ["Pt", "Sa", "Ça", "Pe", "Cu", "Ct", "Pa"].forEach((lab) => {
      const h = document.createElement("div");
      h.className = "cal-dow";
      h.textContent = lab;
      grid.appendChild(h);
    });
    const first = new Date(y, m, 1);
    let start = (first.getDay() + 6) % 7;
    const dim = new Date(y, m + 1, 0).getDate();
    const dotted = daysWithItems(y, m);
    for (let i = 0; i < start; i++) {
      const cell = document.createElement("div");
      cell.className = "cal-cell empty";
      grid.appendChild(cell);
    }
    for (let day = 1; day <= dim; day++) {
      const key = `${y}-${String(m + 1).padStart(2, "0")}-${String(day).padStart(2, "0")}`;
      const cell = document.createElement("button");
      cell.type = "button";
      cell.className = "cal-cell" + (dotted.has(key) ? " has" : "") + (selectedDay === key ? " sel" : "");
      cell.innerHTML = `<span>${day}</span>${dotted.has(key) ? '<i class="dot"></i>' : ""}`;
      cell.addEventListener("click", () => {
        selectedDay = key;
        renderCalendar();
        const dayList = document.getElementById("calDayList");
        dayList.innerHTML = "";
        notesForDay(key).forEach((n) => renderNoteButton(n, dayList));
      });
      grid.appendChild(cell);
    }
  }

  function showTab(name) {
    document.querySelectorAll(".tab").forEach((t) => t.classList.toggle("active", t.dataset.tab === name));
    document.getElementById("panelTasks").classList.toggle("hidden", name !== "tasks");
    document.getElementById("panelCalendar").classList.toggle("hidden", name !== "calendar");
    document.getElementById("panelAsk").classList.toggle("hidden", name !== "ask");
    if (name === "tasks") renderTasks();
    if (name === "calendar") renderCalendar();
  }

  function openPlayer(n) {
    activeNote = n;
    player.hidden = false;
    document.getElementById("playerId").textContent = displayLabel(n);
    document.getElementById("playerTag").textContent = n.tag || "note";
    const bits = [
      F.formatWhen(n.createdAt),
      fmtDuration(n.durationMs),
      statusLabel[n.status] || n.status,
    ];
    if (n.type) bits.push(typeLabel[n.type] || n.type);
    if (n.dueAt) bits.push(`son: ${F.formatWhen(n.dueAt)}`);
    document.getElementById("playerMeta").textContent = bits.join(" · ");
    document.getElementById("playerTranscript").textContent = n.transcript || "—";
    const err = document.getElementById("playerError");
    const errText = [n.error, n.organizeError].filter(Boolean).join(" · ");
    err.textContent = errText;
    err.classList.toggle("hidden", !errText);

    doneRow.classList.toggle("hidden", n.type !== "task");
    doneCb.checked = !!n.done;

    markerList.innerHTML = "";
    const audio = document.getElementById("detailAudio");
    (n.markers || []).forEach((m) => {
      const li = document.createElement("li");
      const b = document.createElement("button");
      b.type = "button";
      b.textContent = `${(m.tMs / 1000).toFixed(1)}s` + (m.label ? ` — ${m.label}` : "");
      b.addEventListener("click", (ev) => {
        ev.preventDefault();
        audio.currentTime = (m.tMs || 0) / 1000;
        audio.play().catch(() => {});
      });
      li.appendChild(b);
      markerList.appendChild(li);
    });

    const audioUrl = n._audioUrl || `/api/notes/${encodeURIComponent(n.id)}/audio`;
    audio.removeAttribute("src");
    if (n._hasAudio === false) {
      audio.classList.add("hidden");
    } else {
      audio.classList.remove("hidden");
      audio.src = audioUrl;
    }
    player.scrollIntoView({ behavior: "smooth", block: "nearest" });
  }

  function closePlayer() {
    const audio = document.getElementById("detailAudio");
    audio.pause();
    player.hidden = true;
    activeNote = null;
  }

  async function deleteActive() {
    if (!activeNote) return;
    if (!confirm(`${displayLabel(activeNote)} silinsin mi?`)) return;
    if (fixtureMode) {
      notes = notes.filter((n) => n.id !== activeNote.id);
      closePlayer();
      renderNotes();
      renderTasks();
      return;
    }
    try {
      const res = await fetch(`/api/notes/${encodeURIComponent(activeNote.id)}`, {
        method: "DELETE",
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      notes = notes.filter((n) => n.id !== activeNote.id);
      closePlayer();
      renderNotes();
      renderTasks();
    } catch (e) {
      alert(`Silinemedi: ${e.message}`);
    }
  }

  async function setDone(done) {
    if (!activeNote || activeNote.type !== "task") return;
    if (fixtureMode) {
      activeNote.done = done;
      renderNotes();
      renderTasks();
      return;
    }
    try {
      const res = await fetch(`/api/notes/${encodeURIComponent(activeNote.id)}/done`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ done }),
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      activeNote.done = done;
      renderNotes();
      renderTasks();
    } catch (e) {
      doneCb.checked = !done;
      alert(`Güncellenemedi: ${e.message}`);
    }
  }

  async function runAsk() {
    const q = document.getElementById("askInput").value.trim();
    const out = document.getElementById("askOut");
    if (!q) return;
    if (fixtureMode) {
      out.textContent = "(fixture) Cihazda features.ask açıkken API kullanılır.";
      return;
    }
    out.textContent = "…";
    const res = await fetch("/api/ask", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ question: q }),
    });
    const data = await res.json().catch(() => ({}));
    out.textContent = res.ok ? data.answer || data.text || "" : data.error || res.statusText;
  }

  async function runBrief(range) {
    const out = document.getElementById("briefOut");
    if (fixtureMode) {
      out.textContent = `(fixture) ${range} briefing — cihazda features.briefing açın.`;
      return;
    }
    out.textContent = "…";
    const res = await fetch("/api/briefing", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ range }),
    });
    const data = await res.json().catch(() => ({}));
    out.textContent = res.ok ? data.text || "" : data.error || res.statusText;
  }

  async function loadConfig() {
    try {
      const res = await fetch("/api/config", { cache: "no-store" });
      if (res.ok) {
        const data = await res.json();
        F.setConfig(data.ui || {});
        features = data.features || features;
        return;
      }
    } catch {
      /* fixture defaults */
    }
    F.setConfig({ language: "tr", dateFormat: "DMY", timeFormat: "H24" });
    features = { ask: true, briefing: true };
  }

  async function loadHealth() {
    try {
      const res = await fetch("/api/health", { cache: "no-store" });
      if (!res.ok) return null;
      return await res.json();
    } catch {
      return null;
    }
  }

  async function load() {
    await loadConfig();
    const health = await loadHealth();
    let data = null;
    fixtureMode = false;
    try {
      const res = await fetch("/api/notes", { cache: "no-store" });
      if (res.ok) data = await res.json();
    } catch {
      /* */
    }
    if (!data) {
      const res = await fetch("fixtures/notes.json", { cache: "no-store" });
      data = await res.json();
      fixtureMode = true;
      for (const n of data.notes || []) n._hasAudio = false;
    }
    notes = data.notes || [];
    const d = data.device || {};
    const parts = [];
    parts.push(d.name || "INote");
    if (health && health.ip) parts.push(health.ip);
    parts.push(d.wifi || (health && health.ip) ? "Wi-Fi bağlı" : "Wi-Fi yok");
    if (typeof d.batteryPercent === "number") parts.push(`Batarya %${d.batteryPercent}`);
    if (d.pendingCount) parts.push(`${d.pendingCount} bekleyen`);
    if (fixtureMode) parts.push("fixture");
    statusEl.textContent = parts.join(" · ");

    const askOn = !!features.ask;
    const briefOn = !!features.briefing;
    document.getElementById("askBox").classList.toggle("hidden", !askOn);
    document.getElementById("briefBox").classList.toggle("hidden", !briefOn);
    document.getElementById("askDisabled").classList.toggle("hidden", askOn || briefOn);

    renderNotes();
    renderTasks();
    renderCalendar();
  }

  searchEl.addEventListener("input", () => {
    query = searchEl.value;
    renderNotes();
    renderTasks();
  });

  document.querySelectorAll(".tab").forEach((t) => t.addEventListener("click", () => showTab(t.dataset.tab)));

  document.querySelectorAll("[data-filter]").forEach((chip) => {
    chip.addEventListener("click", () => {
      document.querySelectorAll("[data-filter], [data-filter-tag]").forEach((c) => c.classList.remove("active"));
      chip.classList.add("active");
      typeFilter = chip.dataset.filter;
      tagFilter = "";
      renderNotes();
    });
  });
  document.querySelectorAll("[data-filter-tag]").forEach((chip) => {
    chip.addEventListener("click", () => {
      document.querySelectorAll("[data-filter], [data-filter-tag]").forEach((c) => c.classList.remove("active"));
      chip.classList.add("active");
      tagFilter = chip.dataset.filterTag;
      typeFilter = "all";
      renderNotes();
    });
  });
  document.querySelectorAll("[data-task-filter]").forEach((chip) => {
    chip.addEventListener("click", () => {
      document.querySelectorAll("[data-task-filter]").forEach((c) => c.classList.remove("active"));
      chip.classList.add("active");
      taskFilter = chip.dataset.taskFilter;
      renderTasks();
    });
  });
  document.getElementById("calPrev").addEventListener("click", () => {
    calCursor.setMonth(calCursor.getMonth() - 1);
    renderCalendar();
  });
  document.getElementById("calNext").addEventListener("click", () => {
    calCursor.setMonth(calCursor.getMonth() + 1);
    renderCalendar();
  });
  doneCb.addEventListener("change", () => setDone(doneCb.checked));
  document.getElementById("askBtn").addEventListener("click", () => runAsk().catch((e) => alert(e.message)));
  document.getElementById("briefDay").addEventListener("click", () => runBrief("day").catch((e) => alert(e.message)));
  document.getElementById("briefWeek").addEventListener("click", () => runBrief("week").catch((e) => alert(e.message)));
  document.getElementById("playerClose").addEventListener("click", closePlayer);
  document.getElementById("deleteBtn").addEventListener("click", () => deleteActive());

  load().catch((err) => {
    statusEl.textContent = `Yüklenemedi: ${err.message}`;
  });
})();

(() => {
  const listEl = document.getElementById("noteList");
  const emptyEl = document.getElementById("empty");
  const searchEl = document.getElementById("search");
  const statusEl = document.getElementById("deviceStatus");
  const detail = document.getElementById("detail");
  const chips = [...document.querySelectorAll(".chip")];
  const doneRow = document.getElementById("doneRow");
  const doneCb = document.getElementById("detailDone");

  let notes = [];
  let typeFilter = "all";
  let query = "";
  let fixtureMode = false;
  let activeNote = null;

  const statusLabel = {
    ready: "hazır",
    pending: "bekleyen",
    transcribing: "işleniyor",
    recording: "kayıt",
    error: "hata",
  };

  const typeLabel = {
    note: "not",
    task: "görev",
    appointment: "randevu",
  };

  function formatWhen(iso) {
    if (!iso) return "";
    try {
      return new Date(iso).toLocaleString("tr-TR", {
        dateStyle: "medium",
        timeStyle: "short",
      });
    } catch {
      return iso;
    }
  }

  function filtered() {
    const q = query.trim().toLocaleLowerCase("tr");
    return notes.filter((n) => {
      if (typeFilter === "pending" || typeFilter === "error") {
        if (n.status !== typeFilter) return false;
      } else if (typeFilter !== "all") {
        if ((n.type || "") !== typeFilter) return false;
      }
      if (!q) return true;
      const hay = `${n.transcript || ""} ${n.title || ""} ${n.id} ${n.error || ""}`.toLocaleLowerCase("tr");
      return hay.includes(q);
    });
  }

  function render() {
    const items = filtered();
    listEl.innerHTML = "";
    emptyEl.classList.toggle("hidden", items.length > 0);
    for (const n of items) {
      const li = document.createElement("li");
      const btn = document.createElement("button");
      btn.type = "button";
      btn.className = "note";
      const typeBit = n.type ? ` · ${typeLabel[n.type] || n.type}` : "";
      const dueBit = n.dueAt ? ` · ${formatWhen(n.dueAt)}` : "";
      const doneBit = n.type === "task" && n.done ? " ✓" : "";
      btn.innerHTML = `
        <div class="note-head">
          <span>${formatWhen(n.createdAt)}${typeBit}${dueBit}${doneBit}</span>
          <span class="status ${n.status}">${statusLabel[n.status] || n.status}</span>
        </div>
        <p class="preview">${escapeHtml(previewText(n))}</p>
      `;
      btn.addEventListener("click", () => openDetail(n));
      li.appendChild(btn);
      listEl.appendChild(li);
    }
  }

  function previewText(n) {
    if (n.title) return n.title;
    if (n.transcript) return n.transcript;
    if (n.status === "pending") return "Transkripsiyon bekleniyor…";
    if (n.status === "error") return n.error || "Hata";
    return "(metin yok)";
  }

  function escapeHtml(s) {
    return String(s)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function openDetail(n) {
    activeNote = n;
    document.getElementById("detailTitle").textContent = n.title || n.id;
    const bits = [
      formatWhen(n.createdAt),
      `${Math.round((n.durationMs || 0) / 1000)} sn`,
      statusLabel[n.status] || n.status,
    ];
    if (n.type) bits.push(typeLabel[n.type] || n.type);
    if (n.dueAt) bits.push(`son: ${formatWhen(n.dueAt)}`);
    document.getElementById("detailMeta").textContent = bits.join(" · ");
    document.getElementById("detailTranscript").textContent = n.transcript || "—";
    const err = document.getElementById("detailError");
    const errText = [n.error, n.organizeError].filter(Boolean).join(" · ");
    err.textContent = errText;
    err.classList.toggle("hidden", !errText);

    const isTask = n.type === "task";
    doneRow.classList.toggle("hidden", !isTask);
    doneCb.checked = !!n.done;

    const audio = document.getElementById("detailAudio");
    const dl = document.getElementById("detailDownload");
    const audioUrl = n._audioUrl || `/api/notes/${encodeURIComponent(n.id)}/audio`;
    audio.removeAttribute("src");
    dl.href = audioUrl;
    dl.download = `${n.id}.wav`;
    if (n._hasAudio === false) {
      audio.classList.add("hidden");
    } else {
      audio.classList.remove("hidden");
      audio.src = audioUrl;
    }
    detail.showModal();
  }

  async function setDone(done) {
    if (!activeNote || activeNote.type !== "task") return;
    if (fixtureMode) {
      activeNote.done = done;
      const idx = notes.findIndex((x) => x.id === activeNote.id);
      if (idx >= 0) notes[idx].done = done;
      render();
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
      const idx = notes.findIndex((x) => x.id === activeNote.id);
      if (idx >= 0) notes[idx].done = done;
      render();
    } catch (e) {
      doneCb.checked = !done;
      alert(`Güncellenemedi: ${e.message}`);
    }
  }

  async function load() {
    let data = null;
    fixtureMode = false;
    try {
      const res = await fetch("/api/notes", { cache: "no-store" });
      if (res.ok) data = await res.json();
    } catch {
      /* fixture */
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
    if (d.name) parts.push(d.name);
    parts.push(d.wifi ? "Wi-Fi bağlı" : "Wi-Fi yok");
    if (typeof d.batteryPercent === "number") parts.push(`Batarya %${d.batteryPercent}`);
    if (d.pendingCount) parts.push(`${d.pendingCount} bekleyen`);
    if (d.openTasks) parts.push(`${d.openTasks} açık görev`);
    if (fixtureMode) parts.push("fixture");
    statusEl.textContent = parts.join(" · ") || "Yerel portal";
    render();
  }

  searchEl.addEventListener("input", () => {
    query = searchEl.value;
    render();
  });

  chips.forEach((chip) => {
    chip.addEventListener("click", () => {
      chips.forEach((c) => c.classList.remove("active"));
      chip.classList.add("active");
      typeFilter = chip.dataset.filter;
      render();
    });
  });

  doneCb.addEventListener("change", () => setDone(doneCb.checked));

  load().catch((err) => {
    statusEl.textContent = `Yüklenemedi: ${err.message}`;
  });
})();

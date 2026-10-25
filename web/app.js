(() => {
  const listEl = document.getElementById("noteList");
  const emptyEl = document.getElementById("empty");
  const searchEl = document.getElementById("search");
  const statusEl = document.getElementById("deviceStatus");
  const detail = document.getElementById("detail");
  const chips = [...document.querySelectorAll(".chip")];

  let notes = [];
  let statusFilter = "all";
  let query = "";

  const statusLabel = {
    ready: "hazır",
    pending: "bekleyen",
    transcribing: "işleniyor",
    recording: "kayıt",
    error: "hata",
  };

  function formatWhen(iso) {
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
      if (statusFilter !== "all" && n.status !== statusFilter) return false;
      if (!q) return true;
      const hay = `${n.transcript || ""} ${n.id} ${n.error || ""}`.toLocaleLowerCase("tr");
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
      btn.innerHTML = `
        <div class="note-head">
          <span>${formatWhen(n.createdAt)}</span>
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
    document.getElementById("detailTitle").textContent = n.id;
    document.getElementById("detailMeta").textContent =
      `${formatWhen(n.createdAt)} · ${Math.round((n.durationMs || 0) / 1000)} sn · ${statusLabel[n.status] || n.status}`;
    document.getElementById("detailTranscript").textContent = n.transcript || "—";
    const err = document.getElementById("detailError");
    err.textContent = n.error || "";
    err.classList.toggle("hidden", !n.error);

    const audio = document.getElementById("detailAudio");
    const dl = document.getElementById("detailDownload");
    // Fixture mode has no real WAV; device will serve /api/notes/:id/audio
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

  async function load() {
    // Prefer device API; fall back to fixtures for PC prototype
    let data = null;
    try {
      const res = await fetch("/api/notes", { cache: "no-store" });
      if (res.ok) data = await res.json();
    } catch {
      /* fixture fallback */
    }
    if (!data) {
      const res = await fetch("fixtures/notes.json", { cache: "no-store" });
      data = await res.json();
      for (const n of data.notes || []) n._hasAudio = false;
    }
    notes = data.notes || [];
    const d = data.device || {};
    const parts = [];
    if (d.name) parts.push(d.name);
    parts.push(d.wifi ? "Wi-Fi bağlı" : "Wi-Fi yok");
    if (typeof d.batteryPercent === "number") parts.push(`Batarya %${d.batteryPercent}`);
    if (d.pendingCount) parts.push(`${d.pendingCount} bekleyen`);
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
      statusFilter = chip.dataset.status;
      render();
    });
  });

  load().catch((err) => {
    statusEl.textContent = `Yüklenemedi: ${err.message}`;
  });
})();

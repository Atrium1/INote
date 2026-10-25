window.INoteFormat = (() => {
  let cfg = { language: "tr", dateFormat: "DMY", timeFormat: "H24" };

  function setConfig(c) {
    if (!c) return;
    cfg = { ...cfg, ...c };
  }

  function pad(n) {
    return String(n).padStart(2, "0");
  }

  function formatWhen(iso) {
    if (!iso) return "";
    const d = new Date(iso);
    if (Number.isNaN(d.getTime())) return iso;
    const y = d.getFullYear();
    const mo = pad(d.getMonth() + 1);
    const day = pad(d.getDate());
    let datePart;
    if (cfg.dateFormat === "YMD") datePart = `${y}-${mo}-${day}`;
    else if (cfg.dateFormat === "MDY") datePart = `${mo}/${day}/${y}`;
    else datePart = `${day}.${mo}.${y}`;

    let h = d.getHours();
    const mi = pad(d.getMinutes());
    let timePart;
    if (cfg.timeFormat === "H12") {
      const ampm = h >= 12 ? "PM" : "AM";
      h = h % 12 || 12;
      timePart = `${h}:${mi} ${ampm}`;
    } else {
      timePart = `${pad(h)}:${mi}`;
    }
    return `${datePart} ${timePart}`;
  }

  function dayKey(iso) {
    if (!iso) return "";
    const d = new Date(iso);
    if (Number.isNaN(d.getTime())) return iso.slice(0, 10);
    return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}`;
  }

  return { setConfig, formatWhen, dayKey, getConfig: () => ({ ...cfg }) };
})();

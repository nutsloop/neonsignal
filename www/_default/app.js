(function () {
  "use strict";

  var status = document.getElementById("status");
  var title = document.querySelector("h1");
  var palette = ["#00ffd5", "#ff2a6d", "#9d4edd", "#f9f002"];
  var paletteIndex = 0;

  function pad2(value) {
    return String(value).padStart(2, "0");
  }

  function updateStatus() {
    if (!status) {
      return;
    }
    var now = new Date();
    var clock =
      pad2(now.getHours()) + ":" + pad2(now.getMinutes()) + ":" + pad2(now.getSeconds());
    status.textContent = "Signal idle - " + clock;
  }

  function cycleTitle() {
    if (!title) {
      return;
    }
    paletteIndex = (paletteIndex + 1) % palette.length;
    title.style.color = palette[paletteIndex];
  }

  updateStatus();
  setInterval(updateStatus, 1000);
  setInterval(cycleTitle, 2500);

  var links = document.querySelectorAll(".domain-list a");
  links.forEach(function (link) {
    link.addEventListener("mouseenter", function () {
      link.style.textShadow = "0 0 12px rgba(0, 255, 213, 0.6)";
    });
    link.addEventListener("mouseleave", function () {
      link.style.textShadow = "";
    });
  });
})();

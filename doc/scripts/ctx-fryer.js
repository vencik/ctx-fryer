// Alterations of the produced HTML code
window.onload = function () {
    // Make CTX Fryer logo clickable
    var logo = document.getElementById("toctitle");

    logo.onclick = function () {
        document.location.href = "index.html";
    }

    logo.title = "Project home";
}

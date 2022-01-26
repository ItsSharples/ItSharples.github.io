var checkbox = document.querySelector("input[id=menuButton]");
var slideInMenu = document.querySelector("input[id=showMenu]");

checkbox.addEventListener('change', function() {
    slideInMenu.checked = this.checked
});
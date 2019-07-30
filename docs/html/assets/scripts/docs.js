
const HEADER_TEMPLATE = `
<div class="nav top attached" id="header-menu"></div>
<header id="main-header">
    <div class="container">
        <div class="card">
          <div class="card-body py-2">
            <div class="row align-items-center">
              <div class="col-8 col-md-10">
                <input id="header-search-input" type="text" class="form-control form-control-minimal" placeholder="Start typing here ..." aria-label="Search" field_signature="1318412689">
              </div>
              <div class="col-4 col-md-2 text-right">
                <button id="header-search-btn" type="button" class="btn btn-primary btn-lg btn-ico btn-rounded"><i class="fa fa-search"></i></button>
              </div>
            </div>
          </div>
        </div>
    </div>
</header>
`;

const TOP_NAV_TEMPLATE = `
    <nav class="navbar navbar-expand-lg navbar-dark fixed-top bg-primary">
  <a class="navbar-brand bold" href="#">Caption Inspector</a>
  <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarSupportedContent" aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>
  <div class="collapse navbar-collapse" id="navbarSupportedContent">
    <ul class="navbar-nav mr-auto">
      <li class="nav-item active">
        <a class="nav-link" href="#">Home <span class="sr-only">(current)</span></a>
      </li>
      <li class="nav-item">
        <a class="nav-link" href="#">Link</a>
      </li>
      <li class="nav-item dropdown">
        <a class="nav-link dropdown-toggle" href="#" id="navbarDropdown" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
          Dropdown
        </a>
        <div class="dropdown-menu" aria-labelledby="navbarDropdown">
          <a class="dropdown-item" href="#">Action</a>
          <a class="dropdown-item" href="#">Another action</a>
          <div class="dropdown-divider"></div>
          <a class="dropdown-item" href="#">Something else here</a>
        </div>
      </li>
      <li class="nav-item">
        <a class="nav-link disabled" href="#">Disabled</a>
      </li>
    </ul>
    <form class="form-inline my-2 my-lg-0">
      <input class="form-control mr-sm-2" type="search" placeholder="Search" aria-label="Search">
      <button class="btn btn-outline-primary btn-outline-white my-2 my-sm-0 text-white" type="submit">Search</button>
    </form>
  </div>
</nav>`;
const SIDE_NAV_TEMPLATE = `
    <div id="sidebar" class="sidebar">
            <div class="sidebar-header">Documentation</div>
            <div class="sidebar-body" id="sidebar-body">
                <div class="group">
                    <h3>About</h3>
                    <a href="./index.html" class="menu-item active">Home</a>
                    <a href="./artifacts.html" class="menu-item">Artifacts</a>
                </div>
                
                <div class="group">
                    <h3>Interpretation</h3>
                    <a href="./index.html" class="menu-item ">Commands</a>
                    <a href="./artifacts.html" class="menu-item">608</a>
                    <a href="./artifacts.html" class="menu-item">708</a>
                </div>
            </div>
        </div>`;


const SIDE_MENU = `
    <ui class="subMenu sticky" id="DOCS_MENU">
        <a href="#about" class="btn"><i class="fa fa-user-circle"></i>About</a>
        <li class="item" id="account">
            <a href="#account" class="btn"><i class="fa fa-user-circle"></i>Getting Started</a>
            <div class="subMenu">
                <a href="#section-A">item-1</a>
                <a href="#section-B">item-2</a>
                <a href="#section-C">item-3</a>
            </div>
        </li>
        <li class="item" id="about">
            <a href="#about" class="btn"><i class="fa fa-download"></i>Installation</a>
            <div class="subMenu">
                <a href="#section-A">item-1</a>
                <a href="#section-B">item-2</a>
            </div>
        </li>
        <li class="item" id="usage">
            <a href="#support" class="btn"><i class="fa fa-info"></i>Usage</a>
            <div class="subMenu">
                <a href="#section-A">item-1</a>
                <a href="#section-B">item-2</a>
                <a href="#section-C">item-3</a>
                <a href="#section-D">item-4</a>
                <a href="#section-E">item-5</a>
                <a href="#section-F">item-6</a>
            </div>
        </li>
        <li class="item" id="support">
            <a href="#support" class="btn"><i class="fas fa-info"></i>Support</a>
            <div class="subMenu">
                <a href="#section-A">GitHub</a>
                <a href="#section-B">Email</a>
            </div>
        </li>
    </ui>
`;
let $TOP_NAV;
let $HEADER;
let $SIDE_NAV;
(() => {
    document.addEventListener("DOMContentLoaded", () => {
        const jQueryLoaded = window.$ || window.jQuery;
        if (jQueryLoaded) {
            $HEADER = $("#header");
            $SIDE_NAV = $("#navigation");
            let main = (() => {
    
                
                if ($HEADER) { $HEADER.html(HEADER_TEMPLATE); }
                $TOP_NAV = $("#header-menu");
                if ($TOP_NAV) { $TOP_NAV.html(TOP_NAV_TEMPLATE); }
                
                $SIDE_NAV = $("#navigation");
                if ($SIDE_NAV) {
                    $SIDE_NAV.html(SIDE_MENU);
                    $(".sticky").sticky({topSpacing: 390, zIndex: 2, stopper: "#footer"});
                }
    
                
            })();
            $(function () {
                $('[data-toggle="popover"]').popover()
            })
        }
    });
})();

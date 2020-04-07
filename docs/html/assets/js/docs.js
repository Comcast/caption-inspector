$(window).on("load resize", function() {
  const w = $(window).width();
  if (w >= 1200) {
    $("#docs-sidebar")
      .addClass("sidebar-visible")
      .removeClass("sidebar-hidden");
  } else {
    $("#docs-sidebar")
      .addClass("sidebar-hidden")
      .removeClass("sidebar-visible");
  }
});
const LEVEL = { ROOT: 0, PARENT: 1, CHILD: 2, NUB: 3 };
const LEVEL_LABELS = ["ROOT", "PARENT", "CHILD", "NUB"];
const PAGE_URL = (() => {
  let url = window.location.href.toString();
  if (url.includes("#")) {
    url = url.substring(0, url.indexOf("#"));
  }
  return url;
})();
const sectionHeaders =
  '<li class="nav-item section-title" onclick="location.hash=\'{{SECTION}}\'">' +
  '<a class="nav-link scrollto" href="${PAGE_URL}#{{SECTION}}">' +
  '	<span class="theme-icon-holder mr-2">' +
  '		<i class="fas fa-{{ICON}}"></i></span>' +
  '	<div class="outline-indicator">{{ID}}</div>' +
  "	<span>{{LABEL}}</span></a>" +
  "</li>";

const navigationTemplate =
  '<li class="nav-item " onclick="location.hash=\'{{SECTION}}\'">' +
  '  <a class="nav-link scrollto" href="${PAGE_URL}#{{SECTION}}">' +
  '    <div class="outline-indicator">{{ID}}</div>' +
  "    <span>{{LABEL}}</span>" +
  "  </a>" +
  "</li>";

class BuildNav {
  // static JSON = {};
  // static HTML_ARRAY = [];
  // static ROOT = "root";
  // static HEADER = "header";
  // static CHILD = "child";
  static getHeaderTemplate(LABEL, ICON, SECTION) {
    let template = sectionHeaders.toString() + " ";
    let indicator = BuildNav.getId(SECTION);
    template = template.replace(/{{SECTION}}/g, SECTION);
    template = template.replace(/{{SECTION}}/g, SECTION);
    template = template.replace(/{{LABEL}}/g, LABEL);
    template = template.replace(/{{ICON}}/g, ICON || "folder");
    template = template.replace(/{{ID}}/g, indicator);
    return template;
  }
  static getTemplate(LABEL, ICON, SECTION) {
    let template = navigationTemplate.toString() + " ";
    let indicator = BuildNav.getId(SECTION);
    template = template.replace(/{{SECTION}}/g, SECTION);
    template = template.replace(/{{LABEL}}/g, LABEL);
    template = template.replace(/{{ICON}}/g, ICON || "file");
    template = template.replace(/{{ID}}/g, indicator);
    return template;
  }
  static getId(sectionString) {
    sectionString = sectionString
      .toString()
      .trim()
      .replace(/section-/g, "");
    sectionString = sectionString
      .toString()
      .trim()
      .replace(/section./g, "");
    sectionString = sectionString
      .toString()
      .trim()
      .replace(/-/g, ".");
    return sectionString;
  }
  static formatLabel(label, maxLength = 20) {
    if (!label) {
      try {
        //throw "Null Value: BuildNav.formatLabel( label < NULL VALUE )";
      } catch (er) {
        //console.trace(er);
      }
      return "";
    }
    label = label.trim();
    if (label.length > maxLength) {
      label = label.substring(0, maxLength);
    }
    return label;
  }
  static getLabel(element, maxLength = 20) {
    // Prefer title over text.
    if(element){
      let label = $(element).attr("alt") || $(element).attr("data-title") || $(element).attr("data-menu") || $(element).attr("title") || $(element).text();
      if(!label || label === ""){
        //console.log($(element).id)
      }
      return BuildNav.formatLabel(label, maxLength);
    } else {
      return "LABEL NF"
    }
    
  }
  static appendIndex(element, indexId) {
    try {
      if (element && indexId) {
        let html = $(element).html();
        indexId = BuildNav.getId(indexId);
        $(element).html(`
          <a name="${indexId}" id="SECTION_${indexId}"></a>
          <span class="index-id">${indexId}</span>${html}
      `);
      }
    } catch (er) {
      console.warn(er);
    }
  }
  static build(callback) {
    let ROOT = (BuildNav.JSON = new RootElement());
    let introduction = new NavElement(
      null,
      "Introduction",
      "1",
      BuildNav.HEADER
    );
    ROOT.addChild(introduction);
    try {
      $("section.docs-section").each(function(index, ROOT_SECTION) {
        let sectionHeading = $(ROOT_SECTION).find(".section-heading:first");
        if(!sectionHeading){
          sectionHeading = $(ROOT_SECTION);
        }
        let label = BuildNav.getLabel(sectionHeading);
        let id = $(ROOT_SECTION).attr("id");

        let navigationElement = new NavElement(ROOT,label,id,BuildNav.HEADER);
        ROOT.addChild(navigationElement);

        BuildNav.appendIndex(sectionHeading, id);
        let section = BuildNav.getHeaderTemplate(label, "folder", id);
        BuildNav.HTML_ARRAY.push(section);
        /**
         * Now, find all children navigational elements
         */
        $(ROOT_SECTION)
          .find("h2")
          .each(function(index, PARENT_SECTION) {
            let sectionHeading = $(PARENT_SECTION).find(
              ".section-heading:first"
            );
            let label = BuildNav.getLabel(sectionHeading);
            // console.log(" > H2 Header: ", label.trim())
            let id = $(PARENT_SECTION).attr("id");
            BuildNav.appendIndex(sectionHeading, id);
            if (id && id !== "") {
              let section = BuildNav.getTemplate(label, "folder", id);
              BuildNav.HTML_ARRAY.push(section);
            }
          });
        $(this)
          .find("h3")
          .each(function(index, CHILD_SECTION) {
            let label = BuildNav.getLabel(CHILD_SECTION);
            let id = $(CHILD_SECTION).attr("id");
            BuildNav.appendIndex(CHILD_SECTION, id);
            if (id && id !== "") {
              let section = BuildNav.getTemplate(label.trim(), "file", id);

              let childElement = new NavElement(
                navigationElement,
                label,
                id,
                BuildNav.CHILD
              );
              navigationElement.addChild(childElement);

              BuildNav.HTML_ARRAY.push(section);
            }
          });
      });
    } catch (er) {
      console.warn(er);
    }
    // console.log(BuildNav.HTML_ARRAY.join(""));
    // console.log("ROOT", ROOT);
    $("#documentation-outline").html(BuildNav.HTML_ARRAY.join("") || "");
    if (callback) {
      callback();
    }
  }
}
BuildNav.ROOT = "root";
BuildNav.HEADER = "header";
BuildNav.CHILD = "child";
BuildNav.JSON = {};
BuildNav.HTML_ARRAY = [
  '<li class="nav-item section-title"><a class="nav-link scrollto active" href="${PAGE_URL}#section-1"><span class="theme-icon-holder mr-2"><i class="fas fa-folder"></i></span><div class="outline-indicator"></div>Introduction</a></li>'
];

class NavElement {
  constructor(parent, label, id, type = BuildNav.HEADER) {
    this.label = label.toString().trim();
    this.id = id.toString().trim();
    this.type = type.toString().trim();
    this.clicks = 0;
    /**
     * @type NavElement
     */
    this._parent = parent;
    this.children = [];
  }
  get parent() {
    if (this._parent) {
      return this._parent;
    }
    return NavElement.root;
  }
  set parent(parent) {
    if (parent instanceof RootElement) {
      this._parent = null;
    } else if (parent instanceof NavElement) {
      this._parent = parent;
    }
  }
  click() {
    this.clicks++;
    return this;
  }
  count() {
    return this.clicks;
  }
  getType() {
    return this.type
      .toString()
      .trim()
      .toLowerCase();
  }
  hasChildren() {
    return this.children && this.children.length > 0;
  }
  addChild(navElement) {
    let t = this.getType();
    if (t === "header" || t === "root") {
      navElement.parent = this;
      NavElement.root.addChild(navElement);
    }
  }
  static get root() {
    if (!BuildNav.JSON) {
      BuildNav.JSON = new RootElement();
    }
    return BuildNav.JSON;
  }
}

class RootElement extends NavElement {
  constructor() {
    super(null, "ROOT", "0", BuildNav.ROOT);
    this.map = new Map();
    this.parent = null;
    this.children = [];
  }
  click() {
    return this;
  }
  count() {
    return 0;
  }
  size() {
    return this.map.size;
  }
  getType() {
    return BuildNav.ROOT;
  }
  hasChildren() {
    return this.children && this.children.length > 0;
  }
  addChild(navElement) {
    let t = this.getType();
    if (t === "header" || t === "root") {
      this.map.set(navElement.id, navElement);
      // console.warn(
      //   navElement.label + " does not have a parent to add this child to. ",
      //   navElement
      // );
      if (!navElement.parent) {
        this.children.push(navElement);
      } else {
        navElement.parent.children.push(navElement);
      }
    }
  }
  getElement(id) {
    return this.map.get(id);
  }
}

class SearchTags {
  constructor(terms, index) {
    this.index = 0;
    this.score = index || 0;
    this.terms = terms || "";
    this.preferences = {};
    this.results = [];
  }
}

class SearchDocs {
  constructor(searchField) {
    this.field = document.getElementById(searchField);
    this.searches = [];
    this.currentSearch = {};
  }
  search(form) {
    let field = form.elements[0];
    let term = ((field && field.value())||  this.field.value()||"").toString().trim();
    if (term && term !== "") {
      let currentSearch = this.current;
      if (currentSearch && currentSearch.terms !== term) {
        let searchTerm = new SearchTags(term, this.searches.length - 1);
        this.searches.push(searchTerm);
      }
    }
    console.log(this);
  }
  get current() {
    if (this.searches.length > 0) {
      return this.searches[this.searches.length - 1];
    }
    return null;
  }
  clearRecent() {
    let currentSearch = this.current;
    this.searches = [];
    if (currentSearch) {
      this.searches.push(currentSearch);
    }
  }
}

let searchDocs;
$(document).ready(function() {
  BuildNav.build(() => {
    $("#docs-sidebar-toggler").on("click", function() {
      if ($("#docs-sidebar").hasClass("sidebar-visible")) {
        $("#docs-sidebar")
          .removeClass("sidebar-visible")
          .addClass("sidebar-hidden");
      } else {
        $("#docs-sidebar")
          .removeClass("sidebar-hidden")
          .addClass("sidebar-visible");
      }
    });
    /* ====== Activate scrollspy menu ===== */
    $("body").scrollspy({ target: "#docs-nav", offset: 100 });
    /* ===== Smooth scrolling ====== */
    $("#docs-sidebar a.scrollto").on("click", function(e) {
      //store hash
      var target = this.hash;
      e.preventDefault();
      $("body").scrollTo(target, 800, { offset: -69, axis: "y" });
      //Collapse sidebar after clicking
      if (
        $("#docs-sidebar").hasClass("sidebar-visible") &&
        $(window).width() < 1200
      ) {
        $("#docs-sidebar")
          .removeClass("sidebar-visible")
          .addClass("slidebar-hidden");
      }
    });
    /* wmooth scrolling on page load if URL has a hash */
    if (window.location.hash) {
      var urlhash = window.location.hash;
      $("body").scrollTo(urlhash, 800, { offset: -69, axis: "y" });
    }
  });
  searchDocs = new SearchDocs('searchField1');
  /* Bootstrap lightbox */
  /* Ref: http://ashleydw.github.io/lightbox/ */
  $(document).delegate('*[data-toggle="lightbox"]', "click", function(e) {
    e.preventDefault();
    $(this).ekkoLightbox();
  });
  $('[data-toggle="tooltip"]').tooltip();
  $(document).ready(function() {
    $("ol.bold-numbers li").wrapInner("<span class='normal' />")
  })
});

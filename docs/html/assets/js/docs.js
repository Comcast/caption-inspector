$(window).on('load resize', function () {
    var w = $(window).width();
    if (w >= 1200) {
        $('#docs-sidebar')
            .addClass('sidebar-visible')
            .removeClass('sidebar-hidden');
    } else {
        $('#docs-sidebar')
            .addClass('sidebar-hidden')
            .removeClass('sidebar-visible');
    }
});
const LEVEL = {ROOT: 0, PARENT: 1, CHILD: 2, NUB: 3}
const LEVEL_LABELS = ["ROOT", "PARENT", "CHILD", "NUB"];

class OutlineElement {
    constructor(parent, label, id) {
        this.parent = parent || this;
        this.isRoot = (id === "root");
        this.id = id || "";
        this.label = label || "";
        this.level = 0;
        this.children = null;
        if (this.parent) {
            this.level = parent.level + 1;
        }
        if (level && level < 3) {
            this.children = new Map();
        }
    }
    getId() {
    }
    hasChildren() {
        return (this.children && this.children.length > 0);
    }
}
class Outliner {
    constructor() {
        this.outline = new OutlineElement(null, "root", "root", 0);
        this.cursor = this.outline;
    }
    addChild(parent, label, id) {}
    addSibling(parent, label, id) {}
    addGrandchild(parent, label, id) {}
    generateHTML() {
        this.outline.children
            .forEach((parent, key, map) => {
                
                if (parent.hasChildren && parent.hasChildren()) {
                    parent.children.forEach((child, key, map) => {
                      if (child.hasChildren && child.hasChildren()) {
                      
                      }
                  });
                }
            });
    }
    getOutline() {
        this.outline.children.forEach((value, key, map) => {
        });
    }
    getLevel(level) {
        if (LEVEL.ROOT === level) { return "ROOT"; }
    }
}

class BuildNav {
    static sectionHeaders = `
	<li class="nav-item section-title">
		<!-- SECTION HEADER -->
		<a class="nav-link scrollto" href="./docs-page.html#{{SECTION}}">
			<span class="theme-icon-holder mr-2">
				<i class="fas fa-{{ICON}}"></i>
			</span>
			<div class="outline-indicator">{{ID}}</div>
			<label>{{LABEL}}</label>
		</a>
	</li>`;
    static HTML_ARRAY = [];
    static navigationTemplate = `<li class="nav-item ">
		<a class="nav-link scrollto" href="./docs-page.html#{{SECTION}}">
			<span class="theme-icon-holder mr-1">
				<i class="fas fa-{{ICON}}"></i>
			</span>
			<div class="outline-indicator">{{ID}}</div>
			<label>{{LABEL}}</label>
		</a>
	</li>`;
    static getHeaderTemplate(LABEL, ICON, SECTION) {
        let template = BuildNav.sectionHeaders.toString() + " ";
        let indicator = BuildNav.getId(SECTION);
        template = template.replace(/{{SECTION}}/g, SECTION);
        template = template.replace(/{{SECTION}}/g, SECTION);
        template = template.replace(/{{LABEL}}/g, LABEL);
        template = template.replace(/{{ICON}}/g, ICON);
        template = template.replace(/{{ID}}/g, indicator);
        return template;
    }
    static getTemplate(LABEL, ICON, SECTION) {
        let template = BuildNav.navigationTemplate.toString() + " ";
        let indicator = BuildNav.getId(SECTION);
        template = template.replace(/{{SECTION}}/g, SECTION);
        template = template.replace(/{{LABEL}}/g, LABEL);
        template = template.replace(/{{ICON}}/g, ICON);
        template = template.replace(/{{ID}}/g, indicator);
        return template;
    }
    static getId(sectionString) {
        sectionString = sectionString
            .toString().trim().replace(/section-/g, "");
        sectionString = sectionString
            .toString().trim().replace(/section./g, "");
        sectionString = sectionString
            .toString().trim().replace(/-/g, ".");
        return sectionString
    }
    static build(callback = null) {
        // For the current design...
        BuildNav.HTML_ARRAY = [];
        try {
            $("section.docs-section").each(function (index, ROOT_SECTION) {
                let sectionHeading = $(ROOT_SECTION).find(".section-heading:first");
                let sectionLabel = sectionHeading.attr("title") || sectionHeading.text();
                // console.log("Section:  "+ sectionLabel.toString().trim())
                let id = $(ROOT_SECTION).attr("id");
                let section = BuildNav.getHeaderTemplate(sectionLabel.trim(), "check", id);
                BuildNav.HTML_ARRAY.push( section );
                /**
                 * Now, find all children navigational elements
                 */
                $(ROOT_SECTION).find("rh2").each(function (index, PARENT_SECTION) {
                    let sectionHeading = $(PARENT_SECTION).find(".section-heading:first");
                    let sectionLabel = sectionHeading.attr("title") || sectionHeading.text();
                    let label = $(PARENT_SECTION).text();
                    // console.log(" > H2 Header: ", label.trim())
                    let id = $(PARENT_SECTION).attr("id");
                    if (id && id !== "") {
                        let section = BuildNav.getTemplate(label.trim(), "check", id);
                        BuildNav.HTML_ARRAY.push(section);
                    }
                })
                $(this).find("h3").each(function (index, CHILD_SECTION) {
                    let label = $(CHILD_SECTION).text();
                    // console.log(" > > H3: ", label.trim());
                    let id = $(CHILD_SECTION).attr("id");
                    if (id && id !== "") {
                        let section = BuildNav.getTemplate(label.trim(), "file", id);
                        BuildNav.HTML_ARRAY.push(section);
                    }
                })
            })
        } catch (er) {
            console.warn(er);
        }
        console.log(BuildNav.HTML_ARRAY.join(""));
        $("#documentation-outline").html(BuildNav.HTML_ARRAY.join("") || "");
        if (callback && typeof callback === "Function") {
            callback()
        }
    }
}

$(document).ready(function () {
    BuildNav.build(() => {
        $('#docs-sidebar-toggler').on('click', function () {
            if ($('#docs-sidebar').hasClass('sidebar-visible')) {
                $("#docs-sidebar").removeClass('sidebar-visible').addClass('sidebar-hidden');
            } else {
                $("#docs-sidebar").removeClass('sidebar-hidden').addClass('sidebar-visible');
            }
        });
        /* ====== Activate scrollspy menu ===== */
        $('body').scrollspy({target: '#docs-nav', offset: 100});
        /* ===== Smooth scrolling ====== */
        $('#docs-sidebar a.scrollto').on('click', function (e) {
            //store hash
            var target = this.hash;
            e.preventDefault();
            $('body').scrollTo(target, 800, {offset: -69, 'axis': 'y'});
            //Collapse sidebar after clicking
            if ($('#docs-sidebar').hasClass('sidebar-visible') && $(window).width() < 1200) {
                $('#docs-sidebar').removeClass('sidebar-visible').addClass('slidebar-hidden');
            }
        });
        /* wmooth scrolling on page load if URL has a hash */
        if (window.location.hash) {
            var urlhash = window.location.hash;
            $('body').scrollTo(urlhash, 800, {offset: -69, 'axis': 'y'});
        }
    });
    /* Bootstrap lightbox */
    /* Ref: http://ashleydw.github.io/lightbox/ */
    $(document).delegate('*[data-toggle="lightbox"]', 'click', function (e) {
        e.preventDefault();
        $(this).ekkoLightbox();
    });
});

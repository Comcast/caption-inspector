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

const PAGE_URL = (() => {
    let url = window.location.href.toString();
    if (url.includes("#")) {
        url = url.substring(0, url.indexOf("#"));
    }
    return url;
})();

class BuildNav {
    static sectionHeaders = `
	<li class="nav-item section-title">
		<!-- SECTION HEADER -->
		<a class="nav-link scrollto" href="${PAGE_URL}#{{SECTION}}">
			<span class="theme-icon-holder mr-2">
				<i class="fas fa-{{ICON}}"></i>
			</span>
			<div class="outline-indicator">{{ID}}</div>
			<span>{{LABEL}}</span>
		</a>
	</li>`;
    static HTML_ARRAY = [
        `
    <li class="nav-item section-title">
        <a class="nav-link scrollto active" href="${PAGE_URL}#section-1">
            <span class="theme-icon-holder mr-2">
                <i class="fas fa-folder"></i>
            </span>
            Introduction
        </a>
    </li>
    `
    ];
    static navigationTemplate = `
    <li class="nav-item ">
		<a class="nav-link scrollto" href="${PAGE_URL}#{{SECTION}}">
			<div class="outline-indicator">{{ID}}</div>
			<span>{{LABEL}}</span>
		</a>
	</li>`;
    static getHeaderTemplate(LABEL, ICON, SECTION) {
        let template = BuildNav.sectionHeaders.toString() + " ";
        let indicator = BuildNav.getId(SECTION);
        template = template.replace(/{{SECTION}}/g, SECTION);
        template = template.replace(/{{SECTION}}/g, SECTION);
        template = template.replace(/{{LABEL}}/g, LABEL);
        template = template.replace(/{{ICON}}/g, ICON || 'folder');
        template = template.replace(/{{ID}}/g, indicator);
        return template;
    }
    static getTemplate(LABEL, ICON, SECTION) {
        let template = BuildNav.navigationTemplate.toString() + " ";
        let indicator = BuildNav.getId(SECTION);
        template = template.replace(/{{SECTION}}/g, SECTION);
        template = template.replace(/{{LABEL}}/g, LABEL);
        template = template.replace(/{{ICON}}/g, ICON || 'file');
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
    static formatLabel(label, maxLength = 18) {
        if (!label) {
            console.warn("Null Value: BuildNav.formatLabel( label < NULL VALUE )")
            return "";
        }
        label = label.trim();
        if (label.length > maxLength) {
            label = label.substring(0, maxLength)
        }
        return label;
    }
    static getLabel(element, maxLength = 18) {
        // Prefer title over text.
        let label = $(element).attr("title") || $(element).text();
        return BuildNav.formatLabel(label, maxLength)
    }
    static appendIndex(element, indexId) {
        try {
            if (element && indexId) {
                let html = $(element).html();
                indexId = BuildNav.getId(indexId);
                $(element).html(`<span class="index-id">${indexId}</span>${html}`);
            }
        } catch (er) {
            console.warn(er);
        }
    }
    static build(callback = null) {
        try {
            $("section.docs-section").each(function (index, ROOT_SECTION) {
                let sectionHeading = $(ROOT_SECTION).find(".section-heading:first");
                let label = BuildNav.getLabel(sectionHeading);
                // console.log("Section:  "+ sectionLabel.toString().trim())
                let id = $(ROOT_SECTION).attr("id");
                BuildNav.appendIndex(sectionHeading, id);
                let section = BuildNav.getHeaderTemplate(label, "folder", id);
                BuildNav.HTML_ARRAY.push(section);
                /**
                 * Now, find all children navigational elements
                 */
                $(ROOT_SECTION).find("rh2").each(function (index, PARENT_SECTION) {
                    let sectionHeading = $(PARENT_SECTION).find(".section-heading:first");
                    let label = BuildNav.getLabel(sectionHeading);
                    // console.log(" > H2 Header: ", label.trim())
                    let id = $(PARENT_SECTION).attr("id");
                    BuildNav.appendIndex(sectionHeading, id);
                    if (id && id !== "") {
                        let section = BuildNav.getTemplate(label, "folder", id);
                        BuildNav.HTML_ARRAY.push(section);
                    }
                })
                $(this).find("h3").each(function (index, CHILD_SECTION) {
                    let label = $(CHILD_SECTION).text();
                    // console.log(" > > H3: ", label.trim());
                    let id = $(CHILD_SECTION).attr("id");
                    BuildNav.appendIndex(CHILD_SECTION, id);
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
        if (callback) {
            callback();
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
    $('[data-toggle="tooltip"]').tooltip();
});


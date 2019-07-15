
$(window).on('load resize', function() {
    var w = $(window).width();
	if(w >= 1200) {
	    $('#docs-sidebar').addClass('sidebar-visible').removeClass('sidebar-hidden');
	} else {
	    $('#docs-sidebar').addClass('sidebar-hidden').removeClass('sidebar-visible');
	}
});

class BuildNav {
	static sectionHeaders =`
	<li class="nav-item section-title">
		<!-- SECTION HEADER -->
		<a class="nav-link scrollto" href="#{{SECTION}}">
			<span class="theme-icon-holder mr-2">
				<i class="fas fa-{{ICON}}"></i>
			</span>
			<span class="outline-indicator">{{ID}}</span>
			{{LABEL}}
		</a>
	</li>` ;
	static HTML_ARRAY = [];
	static navigationTemplate =`<li class="nav-item ">
		<a class="nav-link scrollto" href="#{{SECTION}}">
			<span class="theme-icon-holder mr-1">
				<i class="fas fa-{{ICON}}"></i>
			</span>
			<span class="outline-indicator">{{ID}}</span>
			{{LABEL}}
		</a>
	</li>` ;
	static getHeaderTemplate(LABEL, ICON, SECTION){
		let template = BuildNav.sectionHeaders.toString() + " ";
		let indicator = BuildNav.getId(SECTION);
		template = template.replace(/{{SECTION}}/g, SECTION);
		template = template.replace(/{{SECTION}}/g, SECTION);
		template = template.replace(/{{LABEL}}/g, 	LABEL);
		template = template.replace(/{{ICON}}/g, 	ICON);
		template = template.replace(/{{ID}}/g, 	indicator);
		return template;
	}
	static getTemplate(LABEL, ICON, SECTION){
		let template = BuildNav.navigationTemplate.toString() + " ";
		let indicator = BuildNav.getId(SECTION);
		template = template.replace(/{{SECTION}}/g, SECTION);
		template = template.replace(/{{LABEL}}/g, 	LABEL);
		template = template.replace(/{{ICON}}/g, 	ICON);
		template = template.replace(/{{ID}}/g, 	indicator);
		return template;
	}
	static getId(sectionString){
		sectionString = sectionString
			.toString().trim().replace(/sextion-/g,"");
		sectionString = sectionString
			.toString().trim().replace(/section./g,"");
		sectionString = sectionString
			.toString().trim().replace(/-/g,".");
		return sectionString
	}
	static build(callback){
		// For the current design...
		BuildNav.HTML_ARRAY = [];
		try {
			$("section.docs-section").each(
				function(index, SECTION_HEADER){
					let label = $(SECTION_HEADER).find(".section-heading:first").text();
					
					// console.log("Section:  ", label.trim())
					let id = $(SECTION_HEADER).attr("id");
					let header = BuildNav.getHeaderTemplate(label.trim(), "check", id);
					BuildNav.HTML_ARRAY.push(header);
					
					$(SECTION_HEADER).find("rh2").each(
						function(index, SECTION_H2){
							let label = $(SECTION_H2).text();
							// console.log(" > H2 Header: ", label.trim())
							let id = $(SECTION_H2).attr("id");
							if(id && id !== ""){
								let section = BuildNav.getTemplate(
									label.trim(),
									"check",
									id);
								BuildNav.HTML_ARRAY.push(section);
							}
						}
					)
					
					$(this).find("h3").each(
						function(index, SECTION_H3){
							let label = $(SECTION_H3).text();
							// console.log(" > > H3: ", label.trim());
							let id = $(SECTION_H3).attr("id");
							if(id && id !== ""){
								let section = BuildNav.getTemplate(
									label.trim(), "file",
									id);
								BuildNav.HTML_ARRAY.push(section);
							}
						}
					)
					
				}
			)
		} catch (er) {
			console.warn(er);
		}
		console.log(BuildNav.HTML_ARRAY.join(""));
		$("#navigation-menu").html = BuildNav.HTML_ARRAY.join("") || "";
		if(callback){
			callback()
		}
	}
}


$(document).ready(function() {
	BuildNav.build(()=>{
		$('#docs-sidebar-toggler').on('click', function(){
			if ( $('#docs-sidebar').hasClass('sidebar-visible') ) {
				$("#docs-sidebar").removeClass('sidebar-visible').addClass('sidebar-hidden');
			} else {
				$("#docs-sidebar").removeClass('sidebar-hidden').addClass('sidebar-visible');
			}
		});
		
		/* ====== Activate scrollspy menu ===== */
		$('body').scrollspy({target: '#docs-nav', offset: 100});
		/* ===== Smooth scrolling ====== */
		$('#docs-sidebar a.scrollto').on('click', function(e){
			//store hash
			var target = this.hash;
			e.preventDefault();
			$('body').scrollTo(target, 800, {offset: -69, 'axis':'y'});
			//Collapse sidebar after clicking
			if ($('#docs-sidebar').hasClass('sidebar-visible') && $(window).width() < 1200){
				$('#docs-sidebar').removeClass('sidebar-visible').addClass('slidebar-hidden');
			}
		});
		/* wmooth scrolling on page load if URL has a hash */
		if(window.location.hash) {
			var urlhash = window.location.hash;
			$('body').scrollTo(urlhash, 800, {offset: -69, 'axis':'y'});
		}
	});
	
	/* Bootstrap lightbox */
    /* Ref: http://ashleydw.github.io/lightbox/ */
    $(document).delegate('*[data-toggle="lightbox"]', 'click', function(e) {
        e.preventDefault();
        $(this).ekkoLightbox();
    });
});

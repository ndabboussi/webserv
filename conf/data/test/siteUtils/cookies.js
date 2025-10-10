
fetch("/siteUtils/sidebar.html")
        .then(res => res.text())
        .then(html => {
          document.getElementById("sidebar-container").innerHTML = html;
        });

function setCookie(name, value, days = 0.2)
{
	const expires = new Date(Date.now() + days*864e5).toUTCString();
	document.cookie = `${name}=${value}; path=/; expires=${expires}`;
}

function getCookie(name)
{
	return document.cookie
		.split('; ')
		.find(row => row.startsWith(name + '='))
		?.split('=')[1];
}

function eraseCookie(name)
{
	document.cookie = `${name}=; Max-Age=0; path=/`;
}

function setTheme(theme)
{
	setCookie('theme', theme);
}

function setFontSize(size)
{
	setCookie('fontsize', size);
}

function resetPreferences()
{
	eraseCookie('theme');
	eraseCookie('fontsize');
}

function togglerightSidebar()
{
	document.body.classList.toggle('rightsidebar-visible');
}

function toggleSidebar()
{
	document.body.classList.toggle('sidebar-visible');
}


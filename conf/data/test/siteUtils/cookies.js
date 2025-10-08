
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
	//document.body.dataset.theme = theme;
	setCookie('theme', theme);
}

function setFontSize(size)
{
	//document.body.dataset.fontsize = size;
	setCookie('fontsize', size);
}

function resetPreferences()
{
	eraseCookie('theme');
	eraseCookie('fontsize');
	//document.body.dataset.theme = 'light';
	//document.body.dataset.fontsize = 'normal';
}

function toggleSidebar()
{
	document.body.classList.toggle('sidebar-visible');
}

window.onload = function()
{
	const theme = getCookie('theme') || 'light';
	const fontsize = getCookie('fontsize') || 'normal';
	//document.body.dataset.theme = theme;
	//document.body.dataset.fontsize = fontsize;
};

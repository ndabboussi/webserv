fetch("/siteUtils/sidebar.html")
  .then(res => res.text())
  .then(html => {
    document.getElementById("sidebar-container").innerHTML = html;

    initSidebar();
  })
  .catch(err => console.error("Erreur lors du chargement de la sidebar :", err));

function initSidebar() {
  const loginSection = document.getElementById("loginSection");
  const profileSection = document.getElementById("profileSection");
  if (!loginSection || !profileSection) {
    console.error("Impossible d’initialiser la sidebar : éléments manquants.");
    return;
  }


  const authToken = getCookie("auth_token");

  if (authToken) {
    fetch("/me")
      .then(res => {
        if (!res.ok) throw new Error("Impossible de récupérer le profil");
        return res.json();
      })
      .then(profile => displayProfile(profile))
      .catch(err => {
        console.error("Erreur de profil :", err);
        showLoginMenu();
      });
  } else {
    showLoginMenu();
  }

  function showLoginMenu() {
    loginSection.innerHTML = `
      <h2>Login</h2>
      <button id="loginBtn">Login</button>
      <hr>
      <button id="registerBtn">Register</button>
    `;

    const loginBtn = document.getElementById("loginBtn");
    const registerBtn = document.getElementById("registerBtn");

    loginBtn.addEventListener("click", showLoginForm);
    registerBtn.addEventListener("click", showRegisterForm);
  }

  function showLoginForm() {
    loginSection.innerHTML = `
      <h2>Login</h2>
      <form id="loginForm" action="/login" method="POST">
        <label>Nom d'utilisateur :</label>
        <input type="text" name="username" required><br>
        <label>Mot de passe :</label>
        <input type="password" name="password" required><br>
        <button type="submit">Se connecter</button>
      </form>
      <button id="backToMenu">⬅ Retour</button>
    `;

    document.getElementById("backToMenu").addEventListener("click", showLoginMenu);
    setupLoginForm();
  }

  function showRegisterForm() {
    loginSection.innerHTML = `
      <h2>Register</h2>
      <form id="registerForm" action="/register" method="POST">
        <label>Username :</label>
        <input type="text" id="username" name="username" required><br>
        <label>Name :</label>
        <input type="text" id="name" name="name" required><br>
        <label>Second name :</label>
        <input type="text" id="second_name" name="second_name" required><br>
        <label>Mot de passe :</label>
        <input type="password" id="password" name="password" required><br>
        <button type="submit">Créer un compte</button>
      </form>
      <button id="backToMenu">⬅ Retour</button>
    `;

    document.getElementById("backToMenu").addEventListener("click", showLoginMenu);
    setupFormHandlers();
  }
}

async function loadProfile(){
  try {
    const profileRes = await fetch("/me", { credentials: "include" });
    if (!profileRes.ok) throw new Error("Impossible de récupérer le profil");
    const profile = await profileRes.json();
    displayProfile(profile);
  } catch (err) {
    console.error(err);
  }
}

function setupLoginForm() {
  const lf = document.getElementById("loginForm");
  if (!lf) return;

  lf.addEventListener("submit", async (e) => {
    e.preventDefault();

    const formData = new URLSearchParams({
      username: lf.username.value,
      password: lf.password.value,
    });

    try {
      const res = await fetch("/login", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: formData.toString(),
        credentials: "include"
      });

      if (!res.ok) throw new Error("Erreur serveur : " + res.status);

      loadProfile();
    } catch (err) {
    console.error(err);
    alert("Erreur de connexion : " + err.message);
  }
  });
}

function setupFormHandlers() {
  const rf = document.getElementById("registerForm");
  if (!rf) return;

  rf.addEventListener("submit", async (e) => {
    e.preventDefault();

    const formData = new URLSearchParams({
      username: rf.username.value,
      name: rf.name.value,
      second_name: rf.second_name.value,
      password: rf.password.value,
    });

    try {
      const res = await fetch("/register", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: formData.toString(),
      });

      if (!res.ok) throw new Error("Erreur serveur : " + res.status);
      const data = await res.json();

      alert("Compte créé avec succès pour " + data.username);
      // retour au menu login
      document.getElementById("backToMenu").click();
    } catch (err) {
      console.error(err);
      alert("Erreur d’inscription : " + err.message);
    }
  });
}

function displayProfile(profile) {
  const loginSection = document.getElementById("loginSection");
  const profileSection = document.getElementById("profileSection");
  if (!profileSection || !loginSection) return;

  // cacher login, montrer profil
  loginSection.style.display = "none";
  profileSection.style.display = "block";

  profileSection.innerHTML = `
    <h2>Bienvenue, ${profile.username}</h2>
    <p>Nom : ${profile.name}</p>
    <p>Prénom : ${profile.second_name}</p>
    <button id="logoutBtn">Se déconnecter</button>
  `;

  document.getElementById("logoutBtn").addEventListener("click", async () => {
    await fetch("/logout", { method: "POST", credentials: "include" });
    eraseCookie("auth_token");

    profileSection.style.display = "none";
    loginSection.style.display = "block";
    initSidebar(); 
  });
}


function setCookie(name, value, days = 0.2) {
  const expires = new Date(Date.now() + days * 864e5).toUTCString();
  document.cookie = `${name}=${value}; path=/; expires=${expires}`;
}

function getCookie(name) {
  return document.cookie
    .split("; ")
    .find(row => row.startsWith(name + "="))
    ?.split("=")[1];
}

function eraseCookie(name) {
  document.cookie = `${name}=; Max-Age=0; path=/`;
}

function setTheme(theme) {
  setCookie("theme", theme);
}

function setFontSize(size) {
  setCookie("fontsize", size);
}

function resetPreferences() {
  eraseCookie("theme");
  eraseCookie("fontsize");
}

function togglerightSidebar() {
  document.body.classList.toggle("rightsidebar-visible");
}

function toggleSidebar() {
  document.body.classList.toggle("sidebar-visible");
}

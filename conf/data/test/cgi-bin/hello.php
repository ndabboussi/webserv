#!/usr/bin/env php
<?php
header("Content-Type: text/html");
echo "<html><body>";
echo "<h1>PHP CGI Test</h1>";
echo "<p>Hello from PHP!</p>";
echo "<h3>_GET params:</h3><pre>";
print_r($_GET);
echo "</pre>";
echo "<h3>_POST params:</h3><pre>";
print_r($_POST);
echo "</pre>";
echo "</body></html>";
?>

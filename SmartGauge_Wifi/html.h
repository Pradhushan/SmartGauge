#ifndef HTML_H
#define HTML_H

#include <pgmspace.h>

// HTML page template for water level display
const char dataPageTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Water Level</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #f2f2f2; }
    .container { margin-top: 50px; background-color: #ffffff; border-radius: 8px; padding: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }
    h1 { color: #ff5252; }
    .water-level { font-size: 24px; margin-top: 20px; }
    .tank { width: 200px; height: 400px; background-color: #add8e6; border-radius: 20px 20px 0 0; position: relative; overflow: hidden; margin: 20px auto; display: flex; align-items: center; justify-content: center; }
    .percentage { font-size: 24px; color: #333; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Current Water Level</h1>
    <div class="water-level">
      <p>Water Level: %.2f ft (%.0f%%)</p>
      <div class="tank">
        <div class="percentage"><b>%.0f%%</b></div>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";

// HTML page template for no data available
const char noDataPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>No Data Available</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #f2f2f2; }
    .container { margin-top: 50px; background-color: #ffffff; border-radius: 8px; padding: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }
    h1 { color: #ff5252; }
    .error { color: #ff5252; font-size: 18px; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>No Data Available</h1>
    <p>Please check the sensor or the device alignment.</p>
    <div class="error">
      <p>Error!</p>
    </div>
  </div>
</body>
</html>
)rawliteral";

#endif

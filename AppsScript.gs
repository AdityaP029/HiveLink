function doGet(e) { 
  // Log the incoming request parameters
  Logger.log(JSON.stringify(e));
  
  var result = 'Ok';
  
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  } else {
    // Spreadsheet ID (replace with a placeholder)
    var sheet_id = 'YourGoogleSheetsID';
    // Open the spreadsheet and select the active sheet
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
    // Get the next available row for data entry
    var newRow = sheet.getLastRow() + 1; 
    var rowData = [];
    
    var Curr_Date = new Date();
    rowData[0] = Curr_Date; // Date in column A
    
    // Timezone (replace with a generic or non-specific timezone)
    var Curr_Time = Utilities.formatDate(Curr_Date, "YourTimeZone", "h:mm a");
    rowData[1] = Curr_Time; // Time in column B
    
    // Iterate through the request parameters and process them
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      
      // Process different parameters and populate the respective columns
      switch (param) {
        case 'temperature':
          rowData[2] = value; // Temperature in column C
          result = 'Temperature Written on column C'; 
          break;
        case 'humidity':
          rowData[3] = value; // Humidity in column D
          result += ' ,Humidity Written on column D';
          break;
        case 'syrup':
          rowData[4] = value; // Syrup quantity in column E
          result += ' ,Syrup Written on column E';
          break;  
        case 'weight':
          rowData[5] = value; // Weight in column F
          result += ' ,Weight Written on column F';  
          break; 
        default:
          result = "unsupported parameter";
      }
    }
    
    Logger.log(JSON.stringify(rowData));
    // Set the values in the new row
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  
  // Return the result as a text output
  return ContentService.createTextOutput(result);
}

// Helper function to remove quotes from a value
function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}

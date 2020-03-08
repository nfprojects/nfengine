// Get table rows
var $rows = $('#log tbody tr');

// Get search boxes
var s_time =    $('#search_time');
var s_message = $('#search_message');
var s_srcFile = $('#search_srcFile');

// Get arrays for table columns
var columnsText_time =    [];
var columnsText_message = [];
var columnsText_srcFile = [];

// Search whole table and split it into separate columns for further use
var i = 0;
$.each($rows, function() {
    columnsText_time[i] =    $(this).find('.time'   ).text().replace(/\s+/g, ' ');
    columnsText_message[i] = $(this).find('.message').text().replace(/\s+/g, ' ');
    columnsText_srcFile[i] = $(this).find('.srcFile').text().replace(/\s+/g, ' ');
    i++;
});


// Perform search on each release of a key
$('.search').keyup(function() {
    // Construct regexex based on search boxes
    var val_time =    '^(?=.*\\b' + $.trim(s_time.val()   ).split(/\s+/).join('\\b)(?=.*\\b') + ').*$';
    var val_message = '^(?=.*\\b' + $.trim(s_message.val()).split(/\s+/).join('\\b)(?=.*\\b') + ').*$';
    var val_srcFile = '^(?=.*\\b' + $.trim(s_srcFile.val()).split(/\s+/).join('\\b)(?=.*\\b') + ').*$';
    var reg_time =    RegExp(val_time,    'i');
    var reg_message = RegExp(val_message, 'i');
    var reg_srcFile = RegExp(val_srcFile, 'i');

    // Filter table
    $rows.show().filter(function(index) {
        return (!reg_time.test(columnsText_time[index])       ||
                !reg_message.test(columnsText_message[index]) ||
                !reg_srcFile.test(columnsText_srcFile[index]) );
    }).hide();
});
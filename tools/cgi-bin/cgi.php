<?php

    echo "<html><head><title>Photo Upload CGI</title></head><body>";
    echo "<h1>Photo uploading</h1>";
    echo "<form action='' method='post' enctype='multipart/form-data'>";
    echo "  <label for='name'>Your Name:</label>";
    echo "  <input type='text' name='name'>";
    echo "  <br>";
    echo "  <label for='photo'>Choose a photo:</label>";
    echo "  <input type='file' name='photo'>";
    echo "  <br>";
    echo "  <input type='submit' value='Upload'>";
    echo "</form>";
    echo "</body></html>";

    // Display additional button for delete
    echo "<form action='' method='post' id='deleteForm'>";
    echo "  <input type='hidden' name='action' value='delete'>";
    echo "  <input type='button' value='Delete' onclick='sendDeleteRequest()'>";
    echo "</form>";


if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $filename = getenv('FILENAME');
    $path = "/Users/mmakarov/Documents/webservIntra/uploaded_files/" . $filename;
    $file = fopen($path, 'c');
    while (FALSE !== ($line = fgets(STDIN))) {
        fwrite($file, $line);
    }
    echo "\nSuccessfully uploaded!<br>";
    fclose($file);
}
?>

<script>
    function sendDeleteRequest() {
        // Use JavaScript to send a DELETE request when the "Delete" button is clicked
        fetch('/cgi-bin/cgi.php', {
            method: 'DELETE',
            headers: {
                'Content-Type': 'application/json',
                // Add any other headers as needed
            },
            body: JSON.stringify({
                // Add any data you want to send with the DELETE request
            }),
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            // Parse the response text as HTML and append it to the body
            return response.text();
        })
        .then(responseText => {
            document.body.insertAdjacentHTML('beforeend', responseText);
        })
        .catch(error => console.error('Error:', error));
    }
</script>

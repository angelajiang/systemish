# Extract the payload of each "content". Works for multiple "content" payloads
# per line

# Extract chunks of the form content:"XXXX"
cat snort3-community.rules | grep -Poh 'content:\"[^,;]*\"' > extract_temp_1

# Delete the initial content:"
cat extract_temp_1 | cut -c 10- > extract_temp_2

# Delete the quote at end 
cat extract_temp_2 | rev | cut -c 2- | rev | sort | uniq > extract_temp_3

# Sort by length
cat extract_temp_3 | awk '{ print length, $0  }' | sort -n -s | cut -d" " -f2- > content_strings.txt

# Cleanup
rm extract_temp_*

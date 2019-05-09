cd ~/ecliplse-workspace/HM
git add .
var=$(zenity --entry --text="Insert your comment, it will apear in Github!" --title="Commit comment")
echo $var
git commit -m $var
git push
zenity --info
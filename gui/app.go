package main

import (
	"log"
	"os"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/widget"
	//	"card-games/protos"
	"context"
	"github.com/coder/websocket"
	"time"
	"net/http"
)

func buttonPress() {
	content, err := os.ReadFile("test.binpb")
	if err != nil {
		log.Fatal(err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()

	dialopts := websocket.DialOptions{
		HTTPHeader : map[string][]string{
			http.CanonicalHeaderKey("card-games-hwid"): []string{"testing"},
		},
	}

	c, _, err := websocket.Dial(ctx, "ws://localhost:9001", &dialopts)

	if err != nil {
		log.Fatal(err)
	}

	err = c.Write(ctx, websocket.MessageBinary, content)
	if err != nil {
		log.Fatal(err)
	}

	msgType, response, err := c.Read(ctx)
	if err != nil {
		log.Fatal(err)
	}
	
	log.Println(msgType, response)
}

func main() {
	a := app.New()
	w := a.NewWindow("Hello World")

	content := widget.NewButton("click me", buttonPress)
	w.SetContent(content)
	w.ShowAndRun()
}


package main

import (
	"fmt"
	"github.com/er1cw00/elfkit/go/elfkit"
)

func main() {
	sopath := "../test.out"
	reader := elfkit.NewElfReader(sopath)
	if reader == nil || !reader.IsOpen() {
		panic("create ElfReader fail");
	}
	image := reader.Load()
	if image != nil {
		soname := image.GetSoName()
		fmt.Printf("SoName: %s\n", soname)
	}
	segment := image.GetSegmentByType(elfkit.PT_DYNAMIC);
	if segment != nil {
		fmt.Printf("PT_DYNAMIC: %bv\n", segment);
	}
	return;
}
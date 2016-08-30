; ModuleID = 'test_rec1.cpp'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.11.0"

; Function Attrs: ssp uwtable
define i32 @_Z1fi(i32 %x) #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 %x, i32* %2, align 4
  %3 = load i32* %2, align 4
  %4 = icmp sle i32 %3, 1
  br i1 %4, label %5, label %6

; <label>:5                                       ; preds = %0
  store i32 1, i32* %1
  br label %14

; <label>:6                                       ; preds = %0
  %7 = load i32* %2, align 4
  %8 = sub nsw i32 %7, 1
  %9 = call i32 @_Z1fi(i32 %8)
  %10 = load i32* %2, align 4
  %11 = sub nsw i32 %10, 2
  %12 = call i32 @_Z1fi(i32 %11)
  %13 = add nsw i32 %9, %12
  store i32 %13, i32* %1
  br label %14

; <label>:14                                      ; preds = %6, %5
  %15 = load i32* %1
  ret i32 %15
}

attributes #0 = { ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.2 (tags/RELEASE_352/final)"}

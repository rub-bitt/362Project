/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Feb 7, 2024
  * @brief   ECE 362 Lab 7 student template
  ******************************************************************************
*/

/*******************************************************************************/

// Fill out your username!  Even though we're not using an autotest, 
// it should be a habit to fill out your username in this field now.
const char* username = "rwoudneh";

/*******************************************************************************/ 

#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();

// Uncomment only one of the following to test each step
//#define STEP1
//#define STEP2
//#define STEP3
//#define STEP4

void init_usart5() {
    // TODO
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;

    //set PC12 to AFR2=USART5_TX
    GPIOC->MODER &= ~(GPIO_MODER_MODER12);
    GPIOC->MODER |= GPIO_MODER_MODER12_1; //alternate function

    GPIOC->AFR[1] &= ~(GPIO_AFRH_AFRH4);
    GPIOC->AFR[1] |= 0x2 << GPIO_AFRH_AFRH4_Pos;

    //set PD2 to AFR2=USART5_RX
    GPIOD->MODER &= ~(GPIO_MODER_MODER2);
    GPIOD->MODER |= GPIO_MODER_MODER2_1; //alternate function

    GPIOD->AFR[0] &= ~(GPIO_AFRH_AFRH2);
    GPIOD->AFR[0] |= 0x2 << GPIO_AFRH_AFRH2_Pos;

    //enable rcc clock to USART5 peripheral
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
    
    USART5->CR1 &= ~(1 << 0); //clear UE bit
    USART5->CR1 &= ~(1 << 28); //m[1]: set m[1:0] = 00 to make 8 bit word
    USART5->CR1 &= ~(1 << 12); //m[0]
    USART5->CR2 &= ~(0b11 << 12); //set stop bit to 1 stop bit
    USART5->CR1 &= ~(1 << 10); //disable parity control
    USART5->CR1 &= ~(1 << 15); //set oversampling to 16x
    USART5->BRR = (48000000 / 115200); //set baud rate
    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE); //set transmitter enable and receriver enable
    USART5->CR1 |= USART_CR1_UE; //enable the usart

        //wait for usart to be able to transmit and receive
    while(!(USART5->ISR & USART_ISR_REACK)); //WAIT FOR RE bit to be acknowledged
    while(!(USART5->ISR & USART_ISR_TEACK)); //WAIT FOR TE bit to be acknowledged

}

void init_spi1_slow() {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    //PB3 = SCK, PB4 = MISO, PB5 = MOSI -->AF0 FOR ALL
    GPIOB->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5);
    GPIOB->MODER |= (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1);
    GPIOB->AFR[0] &= (GPIO_AFRL_AFRL3 | GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5);

    //initialize spi
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 &= ~(SPI_CR1_SPE); //CLEAR ENABLE TO CONFIGURE
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI; 
    //ssi on because cs is not inverted on sd card reader(?)
    SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2; //add SSOE BIT?
    SPI1->CR2 &= ~SPI_CR2_DS_3; //add SSOE BIT?

    SPI1->CR2 |= SPI_CR2_FRXTH; //rxne event generated if FIFO has 8 bits
    
    SPI1->CR1 |= SPI_CR1_SPE;
}

void enable_sdcard() {
    GPIOB->BRR |= GPIO_BRR_BR_2;
}

void disable_sdcard() {
    GPIOB->BSRR |= GPIO_BSRR_BS_2;
}

void init_sdcard_io () {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~(GPIO_MODER_MODER2); //set pb2 as output
    GPIOB->MODER |= GPIO_MODER_MODER2_0;

    init_spi1_slow();
    disable_sdcard();
}

void sdcard_io_high_speed() {
    SPI1->CR1 &= ~(SPI_CR1_SPE); //CLEAR ENABLE TO CONFIGURE
    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR1 |= SPI_CR1_BR_0; //set to 001 = f_pclk/4
    SPI1->CR1 |= SPI_CR1_SPE;
}


#ifdef STEP1
int main(void){
    internal_clock();
    init_usart5();
    for(;;) {
        while (!(USART5->ISR & USART_ISR_RXNE)) { }
        char c = USART5->RDR;
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = c;
    }
}
#endif

#ifdef STEP2
#include <stdio.h>

// TODO Resolve the echo and carriage-return problem

int __io_putchar(int c) {
    // TODO
    if (c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE)); //content of TDR register has been transferred
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    while (!(USART5->ISR & USART_ISR_RXNE));
    char c = USART5->RDR; //read what is received
    // TODO
    if(c == '\r'){
        c = '\n';
    }
    //__io_putchar(c);
    return c;
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP3
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c) {
    // TODO Copy from your STEP2
    if (c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO
    int val = line_buffer_getchar();
    return val;
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#define STEP4
#ifdef STEP4

#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"
#include "tty.h"
#include "commands.h"
#include "string.h"
#include "ff.h"

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

FATFS fs_storage;

// static const char *month_name[] = {
//     [1] = "Jan",
//     [2] = "Feb",
//     [3] = "Mar",
//     [4] = "Apr",
//     [5] = "May",
//     [6] = "Jun",
//     [7] = "Jul",
//     [8] = "Aug",
//     [9] = "Sep",
//     [10] = "Oct",
//     [11] = "Nov",
//     [12] = "Dec",
// };

struct commands_t usercmds[] = {
    { "add",    add },
    { "mul",    mul },
    // 🐦 - define the bird command pointer here 
  };
  
//   void add(int argc, char *argv[])
//   {
//     int sum = 0;
//     for(int i=1; i < argc; i++) {
//         sum += strtol(argv[i], 0, 0);
//     }
//     printf("The sum is %d\n", sum);
//   }
  
//   void mul(int argc, char *argv[])
//   {
//     int prod = 1;
//     for(int i=1; i < argc; i++) {
//       prod *= strtol(argv[i], 0, 0);
//     }
//     printf("The product is %d\n", prod);
//   }
  
// void print_error(FRESULT fr, const char *msg)
// {
//     const char *errs[] = {
//             [FR_OK] = "Success",
//             [FR_DISK_ERR] = "Hard error in low-level disk I/O layer",
//             [FR_INT_ERR] = "Assertion failed",
//             [FR_NOT_READY] = "Physical drive cannot work",
//             [FR_NO_FILE] = "File not found",
//             [FR_NO_PATH] = "Path not found",
//             [FR_INVALID_NAME] = "Path name format invalid",
//             [FR_DENIED] = "Permision denied",
//             [FR_EXIST] = "Prohibited access",
//             [FR_INVALID_OBJECT] = "File or directory object invalid",
//             [FR_WRITE_PROTECTED] = "Physical drive is write-protected",
//             [FR_INVALID_DRIVE] = "Logical drive number is invalid",
//             [FR_NOT_ENABLED] = "Volume has no work area",
//             [FR_NO_FILESYSTEM] = "Not a valid FAT volume",
//             [FR_MKFS_ABORTED] = "f_mkfs aborted",
//             [FR_TIMEOUT] = "Unable to obtain grant for object",
//             [FR_LOCKED] = "File locked",
//             [FR_NOT_ENOUGH_CORE] = "File name is too large",
//             [FR_TOO_MANY_OPEN_FILES] = "Too many open files",
//             [FR_INVALID_PARAMETER] = "Invalid parameter",
//     };
//     if (fr < 0 || fr >= sizeof errs / sizeof errs[0])
//         printf("%s: Invalid error\n", msg);
//     else
//         printf("%s: %s\n", msg, errs[fr]);
// }

// void cat(int argc, char *argv[])
// {
//     for(int i=1; i<argc; i++) {
//         FIL fil;        /* File object */
//         char line[100]; /* Line buffer */
//         FRESULT fr;     /* FatFs return code */

//         /* Open a text file */
//         fr = f_open(&fil, argv[i], FA_READ);
//         if (fr) {
//             print_error(fr, argv[i]);
//             return;
//         }

//         /* Read every line and display it */
//         while(f_gets(line, sizeof line, &fil))
//             printf(line);
//         /* Close the file */
//         f_close(&fil);
//     }
// }

// // void input(int argc, char *argv[])
// {
//     if (argc != 2) {
//         printf("Specify only one file name to create.");
//         return;
//     }
//     FIL fil;        /* File object */
//     char line[100]; /* Line buffer */
//     FRESULT fr;     /* FatFs return code */
//     fr = f_open(&fil, argv[1], FA_WRITE|FA_CREATE_NEW);
//     if (fr) {
//         print_error(fr, argv[1]);
//         return;
//     }
//     printf("To end input, enter a line with a single '.'\n");
//     for(;;) {
//         fgets(line, sizeof(line)-1, stdin);
//         if (line[0] == '.' && line[1] == '\n')
//             break;
//         int len = strlen(line);
//         if (line[len-1] == '\004')
//             len -= 1;
//         UINT wlen;
//         fr = f_write(&fil, (BYTE*)line, len, &wlen);
//         if (fr)
//             print_error(fr, argv[1]);
//     }
//     f_close(&fil);
// }

// void ls(int argc, char *argv[])
// {
//     FRESULT res;
//     DIR dir;
//     static FILINFO fno;
//     const char *path = "";
//     int info = 0;
//     int i=1;
//     do {
//         if (argv[i][0] == '-') {
//             for(char *c=&argv[i][1]; *c; c++)
//                 if (*c == 'l')
//                     info=1;
//             if (i+1 < argc) {
//                 i += 1;
//                 continue;
//             }
//         } else {
//             path = argv[i];
//         }

//         res = f_opendir(&dir, path);                       /* Open the directory */
//         if (res != FR_OK) {
//             print_error(res, argv[1]);
//             return;
//         }
//         for (;;) {
//             res = f_readdir(&dir, &fno);                   /* Read a directory item */
//             if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
//             if (info) {
//                 printf("%04d-%s-%02d %02d:%02d:%02d %6ld %c%c%c%c%c ",
//                         (fno.fdate >> 9) + 1980,
//                         month_name[fno.fdate >> 5 & 15],
//                         fno.fdate & 31,
//                         fno.ftime >> 11,
//                         fno.ftime >> 5 & 63,
//                         (fno.ftime & 31) * 2,
//                         fno.fsize,
//                         (fno.fattrib & AM_DIR) ? 'D' : '-',
//                         (fno.fattrib & AM_RDO) ? 'R' : '-',
//                         (fno.fattrib & AM_HID) ? 'H' : '-',
//                         (fno.fattrib & AM_SYS) ? 'S' : '-',
//                         (fno.fattrib & AM_ARC) ? 'A' : '-');
//             }
//             if (path[0] != '\0')
//                 printf("%s/%s\n", path, fno.fname);
//             else
//                 printf("%s\n", fno.fname);
//         }
//         f_closedir(&dir);
//         i += 1;
//     } while(i<argc);
// }

// void mount(int argc, char *argv[])
// {
//     FATFS *fs = &fs_storage;
//     if (fs->id != 0) {
//         print_error(FR_DISK_ERR, "Already mounted.");
//         return;
//     }
//     int res = f_mount(fs, "", 1);
//     if (res != FR_OK)
//         print_error(res, "Error occurred while mounting");
// }









// void exec(int argc, char *argv[])
// {
//     //for(int i=0; i<argc; i++)
//     //    printf("%d: %s\n", i, argv[i]);
//     for(int i=0; usercmds[i].cmd != 0; i++)
//         if (strcmp(usercmds[i].cmd, argv[0]) == 0) {
//             usercmds[i].fn(argc, argv);
//             return;
//         }
//     for(int i=0; i<sizeof cmds/sizeof cmds[0]; i++)
//         if (strcmp(cmds[i].cmd, argv[0]) == 0) {
//             cmds[i].fn(argc, argv);
//             return;
//         }
//     printf("%s: No such command.\n", argv[0]);
// }

// void parse_command(char *c)
// {
//     char *argv[20];
//     int argc=0;
//     int skipspace=1;
//     for(; *c; c++) {
//         if (skipspace) {
//             if (*c != ' ' && *c != '\t') {
//                 argv[argc++] = c;
//                 skipspace = 0;
//             }
//         } else {
//             if (*c == ' ' || *c == '\t') {
//                 *c = '\0';
//                 skipspace=1;
//             }
//         }
//     }
//     if (argc > 0) {
//         argv[argc] = "";
//         exec(argc, argv);
//     }
// }

void my_command_shell(void)
{
  char line[100];
  int len = strlen(line);
  puts("This is the STM32 command shell.");
  for(;;) {
      printf("> ");
      fgets(line, 99, stdin);
      line[99] = '\0';
      len = strlen(line);
      if (line[len-1] == '\n')
          line[len-1] = '\0';
      parse_command(line);
  }
}


void enable_tty_interrupt(void) {
    // TODO
    init_usart5();
    USART5->CR1 |= 1 << 5; //SET RXNEIE ON -- turning on interrupt that is raised when receiver has something
    USART5->CR3 |= 1 << 6; //set DMAR on -- receiver dma turned on, "trigger dma operation everytime receive data register becomes not empty"
    NVIC->ISER[0] |= 0b1 << 29; //enabling the interrupt for usart5
    
    // The DMA channel 2 configuration goes here

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;  // First make sure DMA is turned off
    DMA2_Channel2->CMAR = (uint32_t)&serfifo;
    DMA2_Channel2->CPAR = (uint32_t)&USART5->RDR;
    DMA2_Channel2->CNDTR = FIFOSIZE;
    DMA2_Channel2->CCR &= ~(1 << 4); //SET DIR TO READ FROM PERIPHERAL
    DMA2_Channel2->CCR &= ~(DMA_CCR_HTIE | DMA_CCR_TCIE); //CLEAR ENABLES FOR BOTH HALF AND COMPLETE TRANSFER INTERRUPT ENABLES
    
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE); //SET BOTH MSIZE AND PSIZE TO 8 BITS
    DMA2_Channel2->CCR |= DMA_CCR_MINC; //SET MINC
    DMA2_Channel2->CCR &= ~(DMA_CCR_PINC);
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~(DMA_CCR_MEM2MEM);
    DMA2_Channel2->CCR |= 0b11 << 12; //set priority level to very high

    DMA2_Channel2->CCR |= DMA_CCR_EN; 
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar() {
    // TODO
    USART_TypeDef *u = USART5;
    
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
        // while (!(u->ISR & USART_ISR_RXNE))
        //     ;
        // insert_echo_char(u->RDR);
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if (c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    int val = interrupt_getchar();
    return val;
}

// TODO Copy the content for the USART5 ISR here
// TODO Remember to look up for the proper name of the ISR function
void USART3_8_IRQHandler() {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}
int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    command_shell();
}
#endif
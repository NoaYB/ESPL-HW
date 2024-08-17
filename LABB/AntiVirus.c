#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link link;

struct link
{
    link *nextVirus;
    virus *vir;
};

char *sigFileName = "signatures-L";
char *InfFileName = "";
int isLittleOrBIgIndian = -1; // 0 = little, 1 = big

void SetSigFileName()
{
    char newFileName[256];
    printf("Enter new signature file name: ");
    if (fgets(newFileName, sizeof(newFileName), stdin) != NULL)
    {
        // Remove newline character at the end if exists
        size_t len = strlen(newFileName);
        if (len > 0 && newFileName[len - 1] == '\n')
        {
            newFileName[len - 1] = '\0';
        }
        sigFileName = strdup(newFileName);
    }
}

virus *readVirus(FILE *file)
{
    virus *v = (virus *)malloc(sizeof(virus));
    if (fread(&(v->SigSize), sizeof(unsigned short), 1, file) != 1)
    {
        free(v);
        return NULL;
    }

    if (isLittleOrBIgIndian == 1)
    {
        unsigned char *ptr = (unsigned char *)&v->SigSize;
        unsigned char temp = ptr[1];
        ptr[1] = ptr[0];
        ptr[0] = temp;
    }

    fread(v->virusName, 16, 1, file);
    v->sig = (unsigned char *)malloc(v->SigSize);
    fread(v->sig, v->SigSize, 1, file);
    return v;
}

void printVirus(virus *v)
{
    printf("Virus name: %s\n", v->virusName);
    printf("Virus size: %u\n", v->SigSize);
    printf("signature:\n");
    for (int i = 0; i < v->SigSize; i++)
    {
        printf("%02X ", v->sig[i]);
        if ((i + 1) % 20 == 0)
            printf("\n");
    }
    printf("\n");
}

void list_print(link *virus_list, FILE *stream)
{
    link *current = virus_list;
    int count = 0;

    // count the number of links
    while (current != NULL)
    {
        count++;
        current = current->nextVirus;
    }

    // create an array to store the links
    link **links = (link **)malloc(count * sizeof(link *));
    current = virus_list;

    // store the links in the array
    for (int i = 0; i < count; i++)
    {
        links[i] = current;
        current = current->nextVirus;
    }

    // print the links in reverse order
    for (int i = count - 1; i >= 0; i--)
    {
        fprintf(stream, "Virus name: %s\n", links[i]->vir->virusName);
        fprintf(stream, "Virus size: %d\n", links[i]->vir->SigSize);
        fprintf(stream, "signature:\n");
        for (int j = 0; j < links[i]->vir->SigSize; j++)
        {
            fprintf(stream, "%02X ", links[i]->vir->sig[j]);
            if ((j + 1) % 20 == 0)
                fprintf(stream, "\n");
        }
        fprintf(stream, "\n\n");
    }

    free(links);
}

link *list_append(link *virus_list, virus *data)
{
    link *newLink = (link *)malloc(sizeof(link));
    if (newLink == NULL)
    {
        printf("memory allocation is failed\n");
        exit(1);
    }
    newLink->vir = data;
    newLink->nextVirus = virus_list;
    return newLink;
}

void list_free(link *virus_list)
{
    link *current = virus_list;
    while (current != NULL)
    {
        link *temp = current;
        current = current->nextVirus;
        free(temp->vir->sig);
        free(temp->vir);
        free(temp);
    }
    free(sigFileName);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    link *current = virus_list;
    while (current != NULL)
    {
        virus *v = current->vir;
        for (unsigned int i = 0; i <= size - v->SigSize; i++)
        {
            if (memcmp(buffer + i, v->sig, v->SigSize) == 0)
            {
                printf("Starting byte: %u\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus size: %u\n\n", v->SigSize);
            }
        }
        current = current->nextVirus;
    }
}

void load_signatures(link **virus_list)
{
    FILE *file = fopen(sigFileName, "rb");
    if (!file)
    {
        perror("cant open file");
        return;
    }

    char magic[5] = {0};
    fread(magic, 4, 1, file);

    if (strcmp(magic, "VIRL") == 0)
        isLittleOrBIgIndian = 0;
    else if (strcmp(magic, "VIRB") == 0)
        isLittleOrBIgIndian = 1;

    if (isLittleOrBIgIndian == -1)
    {
        fprintf(stderr, "error: this is invalid magic number\n");
        fclose(file);
        return;
    }

    virus *v;
    while ((v = readVirus(file)) != NULL)
    {
        *virus_list = list_append(*virus_list, v);
    }

    fclose(file);
}

void detect_viruses(const char *suspected_file, link *virus_list)
{
    FILE *file = fopen(suspected_file, "rb");
    if (!file)
    {
        perror("cant open suspected file");
        return;
    }

    char buffer[10000];
    unsigned int size = fread(buffer, 1, 10000, file);
    fclose(file);

    detect_virus(buffer, size, virus_list);
}

void neutralize_virus(const char *fileName, int signatureOffset)
{
    FILE *file = fopen(fileName, "r+b");
    if (!file)
    {
        perror("cant open file for neutralizing");
        return;
    }

    fseek(file, signatureOffset, SEEK_SET);
    unsigned char ret = 0xC3;
    fwrite(&ret, 1, 1, file);
    fclose(file);
}

void fix_file_menu(link **virus_list)
{
    FILE *file = fopen(InfFileName, "rb");
    if (!file)
    {
        perror("cant open suspected file");
        return;
    }

    char buffer[10000];
    unsigned int size = fread(buffer, 1, 10000, file);
    fclose(file);

    link *current = *virus_list;
    while (current != NULL)
    {
        virus *v = current->vir;
        for (unsigned int i = 0; i <= size - v->SigSize; i++)
        {
            if (memcmp(buffer + i, v->sig, v->SigSize) == 0)
            {
                neutralize_virus(InfFileName, i);
                printf("Neutralized virus at  %u\n", i);
            }
        }
        current = current->nextVirus;
    }
}

// menu
struct fun_desc
{
    char *name;
    void (*fun)(link **virus_list);
};

void set_sig_file_name_menu(link **virus_list)
{
    SetSigFileName();
}

void load_signatures_menu(link **virus_list)
{
    load_signatures(virus_list);
}

void print_signatures_menu(link **virus_list)
{
    list_print(*virus_list, stdout);
}

void detect_viruses_menu(link **virus_list)
{
    detect_viruses(InfFileName, *virus_list);
}

void quit_menu(link **virus_list)
{
    list_free(*virus_list);
    exit(0);
}

struct fun_desc menu[] = {
    {"Set signatures file name", set_sig_file_name_menu},
    {"Load signatures", load_signatures_menu},
    {"Print signatures", print_signatures_menu},
    {"Detect viruses", detect_viruses_menu},
    {"Fix file", fix_file_menu},
    {"Quit", quit_menu},
    {NULL, NULL}};

void menu_select(link **virus_list)
{
    int menuLen = (sizeof(menu) / sizeof(menu[0])) - 1;

    while (1)
    {
        printf("Select operation from the following menu (ctrl^D for exit): \n");
        for (int i = 0; i < menuLen; i++)
            printf("%d) %s \n", i, menu[i].name);
        printf("Option: \n");

        char input[3];
        int inputToInt = atoi(fgets(input, sizeof(input), stdin));

        if (inputToInt >= 0 && inputToInt < menuLen)
        {
            printf("Within bounds \n");
            menu[inputToInt].fun(virus_list);
            printf("Done\n");
        }
        else
        {
            printf("Not within bounds \n");
            exit(0);
        }
    }
}

int main(int argc, char **argv)
{
    link *virus_list = NULL;

    if (argc > 1)
    {
        InfFileName = argv[1];
    }

    menu_select(&virus_list);

    return 0;
}
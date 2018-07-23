template <typename T>
T template_func_no_usage(T in) { // FunctionDecl: template_func_no_usage
    return in;
}

template <typename T>
T template_func_with_usage(T in) { // 2 matches:
    // FunctionDecl: template_func_with_usage
    //   NOTE: one has TemplateArgumentType 'int'
    //         other has no TemplateArgumentType; type is 'T'
    return in;
}

template <typename T>
class ClassNoUsage { // CXXRecordDecl: ClassNoUsage
private:
    T CNI_dataMember;
};

template <typename T>
class ClassWithUsage { // 4 matches:
    // CXXRecordDecl: ClassWithUsage
    // (2 instances of) CXXConstructorDecl: ClassWithUsage
    //   NOTE: one instance is "noexcept-unevaluated"
    // ClassTemplateSpecializationDecl: ClassWithUsage
private:
    T CWI_dataMember;
};

void foo() { // FunctionDecl: foo
    template_func_with_usage(1); // (causes specialization to be created)
    ClassWithUsage<int> cwu_usage; // (causes specialization to be created)
}
